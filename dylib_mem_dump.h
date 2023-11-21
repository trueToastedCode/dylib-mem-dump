#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <mach-o/dyld.h>

#ifndef DYLIB_MEM_DUMP_H
#define DYLIB_MEM_DUMP_H

#ifdef __cplusplus
	extern "C" {
#endif

/**
 * Finds the index of a dynamically loaded library (dylib) by its image name.
 *
 * @param image_name The image name of the dylib to search for.
 * @param image_index A pointer to a uint32_t variable where the index of the found dylib will be stored.
 *
 * @return Returns true if the dylib with the specified image name is found, and its index is stored
 *         in the image_index parameter. Returns false if the dylib is not found.
 */
bool dylib_find_image_name_index(
    const char* image_name,
    uint32_t* image_index
);

// dylib_base_reference struct represents the reference information for a dynamic library.
struct dylib_base_reference {
    uint64_t static_base_address; // The static base address of the dynamic library.
    uint64_t entry_offset;        // The entry offset within the dynamic library.
};

/**
 * Find the base reference information for a dynamic library.
 *
 * @param image_index The index of the dynamic library in the image list.
 * @param dylib_base_reference Pointer to the dylib_base_reference struct to store the result.
 *
 * @return Returns true if the base reference is found successfully, otherwise returns false.
 */
bool dylib_find_base_reference(
    const uint32_t image_index,
    struct dylib_base_reference* dylib_base_reference
);

/**
 * @brief Find sections in a 64-bit Mach-O dynamic library.
 *
 * Given the index of a dynamic library loaded in the current process and a
 * pointer to a variable for storing the section count, this function returns
 * an array of pointers to section structures.
 *
 * @param image_index The index of the dynamic library.
 * @param section_count A pointer to a variable for storing the section count.
 * @return An array of pointers to section structures, or NULL on failure.
 * @note It is the caller's responsibility to free the memory allocated for the
 * returned array using free().
 */
struct section_64** dylib_find_sections(
    const uint32_t image_index,
    size_t* section_count
);

#ifdef __cplusplus
	}
#endif

#endif // DYLIB_MEM_DUMP_H
