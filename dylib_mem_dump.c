#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include "dylib_mem_dump.h"

bool dylib_find_image_name_index(
    const char* image_name,
    uint32_t* image_index
) {
    // Get the total number of loaded dynamic library images.
    const uint32_t image_count = _dyld_image_count();
    // Iterate through each loaded dynamic library image.
    for (uint32_t i = 0; i < image_count; i++) {
        // Compare the base name of the current dynamic library image's path
        // with the specified image_name.
        if (
            strcmp(
                basename((char*) _dyld_get_image_name(i)),
                image_name
            ) != 0
        ) continue; // If the names do not match, continue to the next iteration.
        // If the names match, set the image_index to the current index and return true.
        *image_index = i;
        return true;
    }
    // If no matching dynamic library image is found, return false.
    return false;
}

bool dylib_find_base_reference(
    const uint32_t image_index,
    struct dylib_base_reference* dylib_base_reference
) {
    // Get the Mach-O header of the dynamic library using its index.
    const struct mach_header_64* header = (const struct mach_header_64*)_dyld_get_image_header(image_index);
    // Check if the header is valid.
    if (!header) return false;
    // Get the first load command in the Mach-O header.
    const struct load_command* loadCmd = (const struct load_command*)(header + 1);
    // Flags to track whether static base address and entry offset are found.
    bool foundStaticBaseAddress = false,
         foundEntryOffset = false;
    // Iterate through all load commands in the Mach-O header.
    for (uint32_t nCmd = 0; nCmd < header->ncmds; nCmd++) {
        // Check the type of the load command.
        if (loadCmd->cmd == LC_SEGMENT_64) {
            // If the load command is a 64-bit segment command,
            // and it's the "__TEXT" segment,
            // extract and store the static base address.
            const struct segment_command_64* segmentCmd = (const struct segment_command_64*)loadCmd;
            if (strcmp(segmentCmd->segname, "__TEXT") == 0) {
                dylib_base_reference->static_base_address = segmentCmd->vmaddr;
                foundStaticBaseAddress = true;
                if (foundEntryOffset) break;
            }
        }
        // Check the type of the load command.
        else if (loadCmd->cmd == LC_MAIN) {
            // If the load command is an entry point command, extract and store the entry offset.
            const struct entry_point_command* entryCmd = (const struct entry_point_command*)loadCmd;
            dylib_base_reference->entry_offset = entryCmd->entryoff;
            foundEntryOffset = true;
            if (foundStaticBaseAddress) break;
        }
        // Move to the next load command.
        loadCmd = (const struct load_command*)((intptr_t)loadCmd + loadCmd->cmdsize);
    }
    // Return true if both static base address and entry offset are found.
    return foundStaticBaseAddress && foundEntryOffset;
}

struct section_64** dylib_find_sections(
    const uint32_t image_index,
    size_t* section_count
) {
    // Get the Mach-O header of the dynamic library using its index.
    const struct mach_header_64* header = (const struct mach_header_64*)_dyld_get_image_header(image_index);
    // Check if the header is valid.
    if (!header) return NULL;
    // Get the first load command in the Mach-O header.
    const struct load_command* loadCmd = (const struct load_command*)(header + 1);
    // Setup array and count
    struct section_64** sections = NULL;
    *section_count = 0;
    // Iterate through all load commands in the Mach-O header.
    for (uint32_t nCmd = 0; nCmd < header->ncmds; nCmd++) {
        // If the load command is a 64-bit segment command,
        if (loadCmd->cmd == LC_SEGMENT_64) {
            const struct segment_command_64* segmentCmd = (const struct segment_command_64*)loadCmd;
            // load nsecs if any present if the current load command
            if (segmentCmd->nsects) {
                // Get the first section
                const struct section_64* section = (const struct section_64*)((intptr_t)loadCmd + sizeof(const struct segment_command_64));
                size_t sectionCountBefore = *section_count;
                *section_count += segmentCmd->nsects;
                // Reallocate memory to add the new sections
                sections = (struct section_64**)realloc(sections, sizeof(const struct section_64) * (*section_count));
                // Make sure the reallocate succeeded
                if (sections == NULL) return NULL;
                // Iterate through all sections
                for (uint32_t nSec = 0; nSec < segmentCmd->nsects; nSec++) {
                    sections[sectionCountBefore + nSec] = (struct section_64*)section;
                    // load the next section
                    section = (const struct section_64*)((intptr_t)section + sizeof(const struct section_64));
                }
            }
        }
        // Move to the next load command.
        loadCmd = (const struct load_command*)((intptr_t)loadCmd + loadCmd->cmdsize);
    }
    return sections;
}

int main() { return 0; }
