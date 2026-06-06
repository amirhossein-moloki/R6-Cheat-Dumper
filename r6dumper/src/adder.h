#pragma once
#include "shared.h"

#define SEPERATOR "||||"
#define OPCODE_SEPERATOR "/|/|/|"
#define SIG_SEPERATOR "|||||"
#define STRING_SEPERATOR "\\|\\|\\|\\|"
#define NAME_SEPERATOR "|/|/|/"
#define ENTRY_SEPERATOR "////"

namespace adder {
	static std::vector<StringReference> do_string_refs(const std::string& name, uint64_t address) {
		auto str_refs = shared::find_string_refs_near(address);
		if (str_refs.empty())
			return str_refs;

		printf("|--0x%" PRIX64 "\n", address);

		size_t max_size = str_refs.size();
		for (size_t i = 0; i < max_size; i++) {
			auto& str_ref = str_refs[i];

			printf("|----%s with offset %lld ", memory + str_ref.string_address, str_ref.offset_to_desired_address);


			// TODO: cache ref counts

			auto refs_to_string = shared::find_xrefs(str_ref.string_address);
			str_ref.reference_count = (int64_t)refs_to_string.size();

			printf("has %lld refs", str_ref.reference_count);

			if (!shared::is_string_ref_unique(&str_ref)) {
				printf(" but is not a unique reference\n");
				str_refs.erase(str_refs.begin() + i);
				i--;
				max_size--;
				continue;
			}

			printf("\n");
		}

		return str_refs;
	}

	static bool do_hex_dump(const std::string& name, uint64_t address, int64_t extra_offset = 0) {
		std::ofstream of;
		of.open("offsets.db", std::ofstream::out | std::ofstream::app);

		of << name << NAME_SEPERATOR << SIG_SEPERATOR;

		for (uint64_t i = address - 0x100; i < address + 0x100; i++)
			of << memory[i];

		if (extra_offset != 0)
			of << SIG_SEPERATOR << extra_offset;

		of << ENTRY_SEPERATOR;
		of.close();

		return true;
	}

	static void run() {
		printf("please enter an offset:\n");
		uint64_t offset;
		std::cin >> std::hex >> offset;

		printf("enter the name of this offset:\n");
		std::string name;
		std::cin.get();
		std::getline(std::cin, name);

		std::vector<StringReference> str_refs;

		if (offset > seg_start(Segment::Data) && offset < seg_end(Segment::Data)) {
			auto offset_refs = shared::find_xrefs(offset);

			if (offset_refs.empty()) {
				printf("unable to find refs\n");
				return;
			}

			printf("found %zu refs to offset\n", offset_refs.size());

			printf("0x%" PRIX64 "\n", offset);

			bool do_break = false;

			for (auto& ref : offset_refs) {
				auto currents_refs = do_string_refs(name, ref);
				for (auto& str_ref : currents_refs) {
					str_refs.push_back(str_ref);

					if (str_ref.reference_count == 1)
						do_break = true;
				}

				if (do_break)
					break;
			}
		}

		else if (offset > seg_start(Segment::Text) && offset < seg_end(Segment::Text)) {
			str_refs = do_string_refs(name, offset);
			if (str_refs.empty())
				do_hex_dump(name, offset);
		}

		// if the offset is out of bounds, assume it is a decryption offset
		else if (offset > seg_end(Segment::Data)) {
			printf("Encryption value detected, enter offset: ");

			uint64_t enc_value = offset;
			uint64_t target_offset;
			std::cin >> std::hex >> target_offset;

			auto refs = shared::find_xrefs(enc_value, 0, 0, 0, false);

			if (refs.empty()) {
				printf("error! no refs to enc value found \n");
				return;
			}

			for (auto& ref : refs) {
				auto currents_refs = do_string_refs(name, ref);
				for (auto& str_ref : currents_refs)
					str_refs.push_back(str_ref);
			}

			if (str_refs.empty()) {
				for (auto& ref : refs) {
						uint64_t offset_ref = shared::find_offset_ref_near(ref, target_offset);

					if (offset_ref != 0) {
						do_hex_dump(name, ref, offset_ref - ref);
						break;
					}
				}
			}
		}

		else {
			printf("error, segment not implemented\n");
			return;
		}

		if (str_refs.empty())
			return;

		auto cmp = [](const StringReference& a, const StringReference& b) {
			if (a.reference_count == b.reference_count)
				return a.offset_to_desired_address <= b.offset_to_desired_address;

			return a.reference_count < b.reference_count;
		};

		std::sort(str_refs.begin(), str_refs.end(), cmp);

		std::ofstream of;
		of.open("offsets.db", std::ofstream::out | std::ofstream::app);

		of << name << NAME_SEPERATOR;

		// if we have strs with one ref then discard any with more
		if (str_refs.front().reference_count == 1) {
			for (size_t i = 1; i < str_refs.size(); i++) {
				if (str_refs[i].reference_count > 1) {
					str_refs.resize(i);
					break;
				}
			}

			for (auto& ref : str_refs)
				of << STRING_SEPERATOR << memory + ref.string_address << SEPERATOR << ref.offset_to_desired_address;
		}

		else {
			for (size_t i = 0; i < str_refs.size(); i++) {
				if (!shared::is_string_ref_unique(&str_refs[i])) {
					str_refs.erase(str_refs.begin() + i);
					i--;
				}
			}

			for (auto& ref : str_refs) {
				of << STRING_SEPERATOR << memory + ref.string_address << SEPERATOR << ref.offset_to_desired_address;
				of << OPCODE_SEPERATOR << ref.rex << ref.instruction << ref.reg;
			}
		}

		of << ENTRY_SEPERATOR;
		of.close();

	}
}