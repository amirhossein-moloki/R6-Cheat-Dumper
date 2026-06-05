#pragma once
#include <Windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <thread>
#include <inttypes.h>
#include <mutex>

extern uint8_t* memory;

#define THREAD_COUNT 6

struct string_reference {
	string_reference(uint64_t string_address = 0, int64_t offset_to_desired_address = 0,
		uint64_t reference_address = 0, int64_t reference_count = 0) : string_address(string_address),
		offset_to_desired_address(offset_to_desired_address), reference_count(reference_count),
		reference_address(reference_address)
	{}

	uint64_t string_address;
	int64_t offset_to_desired_address;
	int64_t reference_count;
	uint64_t reference_address;
	uint8_t rex = 0, instruction = 0, reg = 0;
};

enum class segment : uint8_t {
	text = 0,
	rdata,
	data,
	all
};

static uint64_t seg_start_arr[3] = { 0 };
static uint64_t seg_end_arr[3] = { 0 };

static uint64_t seg_start(segment seg) {
	if (seg == segment::all) return 0;
	return seg_start_arr[static_cast<uint64_t>(seg)];
}
static uint64_t seg_end(segment seg) {
	if (seg == segment::all) return seg_end_arr[2]; // assuming data is last
	return seg_end_arr[static_cast<uint64_t>(seg)];
}

namespace shared {
	static bool discover_segments(uint8_t* header_buffer) {
		PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(header_buffer);
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) return false;

		PIMAGE_NT_HEADERS64 nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(header_buffer + dos_header->e_lfanew);
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE) return false;

		PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(nt_headers);

		bool found_text = false, found_rdata = false, found_data = false;

		for (int i = 0; i < nt_headers->FileHeader.NumberOfSections; i++) {
			IMAGE_SECTION_HEADER& section = sections[i];
			std::string name(reinterpret_cast<char*>(section.Name), 8);

			if (name.find(".text") != std::string::npos) {
				seg_start_arr[static_cast<int>(segment::text)] = section.VirtualAddress;
				seg_end_arr[static_cast<int>(segment::text)] = section.VirtualAddress + section.Misc.VirtualSize;
				found_text = true;
			}
			else if (name.find(".rdata") != std::string::npos) {
				seg_start_arr[static_cast<int>(segment::rdata)] = section.VirtualAddress;
				seg_end_arr[static_cast<int>(segment::rdata)] = section.VirtualAddress + section.Misc.VirtualSize;
				found_rdata = true;
			}
			else if (name.find(".data") != std::string::npos && name.find(".data_") == std::string::npos) {
				// Only take the first .data section or the one exactly named .data
				if (!found_data || strcmp(reinterpret_cast<char*>(section.Name), ".data") == 0) {
					seg_start_arr[static_cast<int>(segment::data)] = section.VirtualAddress;
					seg_end_arr[static_cast<int>(segment::data)] = section.VirtualAddress + section.Misc.VirtualSize;
					found_data = true;
				}
			}
		}

		return found_text && found_rdata && found_data;
	}

	// compares memory with skips
	// Returns true if it matches
	static bool cmp_sig(const uint8_t* sig, uint64_t address, size_t len, const char* skips = "") {
		if (skips == nullptr || skips[0] == '\0') {
			return memcmp(sig, memory + address, len) == 0;
		}

		for (size_t i = 0; i < len; i++) {
			if (skips[i] == '0' && memory[address + i] != sig[i])
				return false;
		}

		return true;
	}

	// searches for instances of a signature
	static std::vector<uint64_t> sig_scan(const uint8_t* sig, size_t len, segment seg = segment::text, const char* skips = "") {
		std::vector<uint64_t> ret;
		if (len == 0) return ret;

		uint64_t start_addr = seg_start(seg);
		uint64_t end_addr = seg_end(seg);
		if (end_addr <= start_addr || end_addr < len) return ret;

		std::mutex mtx;
		uint64_t search_range = end_addr - start_addr - len + 1;

		auto search_loop = [sig, &ret, &mtx, len, skips, start_addr](uint64_t offset, uint64_t size) {
			std::vector<uint64_t> local_ret;
			uint64_t end = offset + size;

			// Optimization: If no skips, use a faster search for the first byte
			if (skips == nullptr || skips[0] == '\0') {
				uint8_t first = sig[0];
				for (uint64_t i = offset; i < end; i++) {
					if (memory[i] == first) {
						if (len == 1 || memcmp(sig + 1, memory + i + 1, len - 1) == 0) {
							local_ret.push_back(i);
						}
					}
				}
			}
			else {
				for (uint64_t i = offset; i < end; i++) {
					if (cmp_sig(sig, i, len, skips)) {
						local_ret.push_back(i);
					}
				}
			}

			if (!local_ret.empty()) {
				std::lock_guard<std::mutex> lock(mtx);
				ret.insert(ret.end(), local_ret.begin(), local_ret.end());
			}
		};

		unsigned int num_threads = std::thread::hardware_concurrency();
		if (num_threads == 0) num_threads = THREAD_COUNT;

		std::vector<std::thread> threads;
		uint64_t chunk_size = search_range / num_threads;

		for (unsigned int i = 0; i < num_threads; i++) {
			uint64_t start = start_addr + i * chunk_size;
			uint64_t size = (i == num_threads - 1) ? (search_range - i * chunk_size) : chunk_size;
			threads.emplace_back(search_loop, start, size);
		}

		for (auto& t : threads) {
			t.join();
		}

		std::sort(ret.begin(), ret.end());
		return ret;
	}

	// checks if the specified address is an instruction (eg. mov, movabs, lea)
	static bool is_address_instruction(uint64_t address, bool relative = true) {
		return (memory[address] == 0x48 || memory[address] == 0x4C) && (relative ? (memory[address + 1] == 0x8B || memory[address + 1] == 0x8D) : (memory[address + 1] == 0xBB || memory[address + 1] == 0xBD || memory[address + 1] == 0xBE || memory[address + 1] == 0xBA || memory[address + 1] == 0xBF || memory[address + 1] == 0xB8));
	}

	// finds references to a variable in the .text segment
	static std::vector<uint64_t> find_xrefs(uint64_t offset, uint8_t rex = 0, uint8_t instruction = 0, uint8_t reg = 0, bool relative = true) {
		std::vector<uint64_t> refs;
		std::mutex mtx;

		// 64 bit size sig for absolute offsets
		uint8_t abs_sig[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
		memcpy(abs_sig, &offset, sizeof(uint64_t));

		auto search_loop = [offset, &refs, &mtx, rex, instruction, reg, relative, &abs_sig](uint64_t start, uint64_t size) {
			uint8_t sig[] = { 0x00, 0x0, 0x0, 0x0 };

			for (uint64_t i = start; i < start + size; i++) {
				uint32_t rel_address;
				if (relative) {
					rel_address = static_cast<uint32_t>(offset - i - 0x7);
					memcpy(sig, &rel_address, sizeof(uint32_t));
				}

				bool opcodes_match = false;

				// no opcodes specified, find rex) (mov / lea)
				if (rex == 0)
					opcodes_match = is_address_instruction(i, relative); // 0xBD = moveabs
				else
					opcodes_match = memory[i] == rex && memory[i + 1] == instruction && memory[i + 2] == reg;

				if (opcodes_match && (relative ? !cmp_sig(sig, i + 3, sizeof(sig)) : !cmp_sig(abs_sig, i + 2, sizeof(abs_sig)))) {
					mtx.lock();
					refs.push_back(i);
					mtx.unlock();
				}
			}
		};

		std::vector<std::thread*> threads;
		threads.resize(THREAD_COUNT);

		uint64_t addr = seg_start(segment::text);
		uint64_t size = (seg_end(segment::text) - addr) / threads.size();

		for (int i = 0; i < threads.size() - 1; i++) {
			threads[i] = new std::thread(search_loop, addr, size);

			addr += size;
		}

		threads.back() = new std::thread(search_loop, addr, seg_end(segment::text) - seg_start(segment::text) - size * (threads.size() - 1));

		for (int i = 0; i < threads.size(); i++) {
			threads[i]->join();
			delete threads[i];
		}

		return refs;
	}

	// guesses how long the instruction, only works if an address related instruction is next so dont use this for others
	static uint8_t get_instruction_length(uint64_t offset) {
		for (uint64_t i = offset + 1; i < offset + 0x10; i++) {
			if (memory[i] == 0x48 || memory[i] == 0x4C)
				return static_cast<uint8_t>(i - offset);
		}
		return 0;
	}

	// extracts an offset from an instruction
	static uint64_t extract_offset(uint64_t offset) {
		auto length = get_instruction_length(offset);

		if (length == 10) {
			uint64_t ret;
			memcpy(&ret, memory + offset + 2, sizeof(uint64_t));

			return ret;
		}
		else if (length == 4) {
			uint16_t ret;
			memcpy(&ret, memory + offset + 2, sizeof(uint16_t));

			return ret;
		}
		else {
			uint32_t ret;
			memcpy(&ret, memory + offset + 3, sizeof(uint32_t));

			return ret;
		}

	}

	// turns relative offset instruction into an absolute offset
	static uint64_t extract_relative_offset(uint64_t offset) {
		// convert relative to absolute
		uint64_t ret;
		ret = offset + extract_offset(offset) + 0x7;

		return ret;
	}

	// checks if there is a valid string at a given offset
	static bool is_string_valid(uint64_t offset) {
		do {
			if (memory[offset] <= 32 || memory[offset] >= 127)
				return false;

			offset++;
		} while (memory[offset] != '\0');

		return true;
	}

	static uint64_t find_offset_ref_near(uint64_t offset, uint64_t desired_offset) {
		for (uint64_t i = offset; i < offset + 0x100; i++) {
			if (extract_offset(i) == desired_offset)
				return i;
		}

		return 0;
	}

	// finds references to .rdata string in the region around a .text section location
	static std::vector<string_reference> find_string_refs_near(uint64_t offset) {
		std::vector<string_reference> refs;

		for (uint64_t i = offset - 0x100; i < offset + 0x100; i++) {
			if (!is_address_instruction(i))
				continue;

			uint64_t address = extract_relative_offset(i);

			// we only want strings in rdata
			if (address < seg_start(segment::rdata) || address > seg_end(segment::rdata))
				continue;

			if (!is_string_valid(address))
				continue;

			refs.push_back(string_reference(address, i - offset, i));

			i += 3;
		}

		return refs;
	}

	// checks if there the opcodes referencing a string are unique
	static bool is_string_ref_unique(string_reference* str_ref) {
		str_ref->rex = memory[str_ref->reference_address];
		str_ref->instruction = memory[str_ref->reference_address + 1];
		str_ref->reg = memory[str_ref->reference_address + 2];

		// if there is only one reference to the string with the specific opcodes
		return find_xrefs(str_ref->string_address, str_ref->rex, str_ref->instruction, str_ref->reg).size() == 1;
	}
}