#ifndef CESH_FILES
#define CESH_FILES

/* Examples:
 * path     - "/usr/bin"
 * filepath - "/usr/bin/CESH"
 * filename - "CESH"
 */

void index_filesystem(void); // Updates MFT records
bool has_filesystem_changed(void); // Returns true if MFT is not up to date


/* Lists a directory's contents (ALWAYS FREE RETURN VALUE AFTER USE)
 * Return codes:
 * <array of contents> - Success
 * 1 - Source directory does not exist
 * 2 - Target parent directory does not exist
 * 3 - Target directory already exists
 * 255 - Not enough memory
 */
char *list_directory_contents(const char *path);


/* Creates an empty directory
 * Return codes:
 * 0 - Success
 * 1 - Target directory already exists
 * 2 - Parent directory does not exist
 * 255 - Not enough memory
 */
uint8_t create_directory(const char *path);


/* Removes an empty directory
 * Return codes:
 * 0 - Success
 * 1 - Target directory is not empty
 * 2 - Target directory does not exist
 */
uint8_t remove_directory(const char *path);


/* Moves directory to new location
 * Return codes:
 * 0 - Success
 * 1 - Source directory does not exist
 * 2 - Target directory already exists
 * 3 - Target parent directory does not exist
 */
uint8_t move_directory(const char *path, uint8_t type, const char *new_path);


/* Copies a directory
 * Return codes:
 * 0 - Success
 * 1 - Source directory does not exist
 * 2 - Target parent directory does not exist
 * 3 - Target directory already exists
 * 255 - Not enough memory
 */
uint8_t copy_directory(const char *fpath, uint8_t type, const char *new_fpath);


/* Creates an empty file at the specified path
 * Return codes:
 * 0 - Success
 * 1 - File already exists
 * 2 - Target directory does not exist
 * 255 - Not enough memory
 */
uint8_t create_file(const char *fname, const char *path);


/* Removes an empty directory
 * Return codes:
 * 0 - Success
 * 1 - File does not exist
 */
uint8_t remove_file(const char *fpath);


/* Moves file to directory
 * Return codes:
 * 0 - Success
 * 1 - File does not exist
 * 2 - Directory does not exist
 * 3 - Target file already exists
 */
uint8_t move_file(const char *fpath, uint8_t type, const char *new_path);


/* Copies a file
 * Return codes:
 * 0 - Success
 * 1 - File does not exist
 * 2 - Target directory does not exist
 * 3 - Target file already exists
 * 255 - Not enough memory
 */
uint8_t copy_file(const char *fpath, uint8_t type, const char *new_fpath);


#endif