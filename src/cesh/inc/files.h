#ifndef CESH_FILES
#define CESH_FILES

/* Keyword examples:
 * path  - "/usr/bin"
 * fpath - "/usr/bin/CESH"
 * fname - "CESH"
 * dname - "/bin"
 *
 * Keep in mind, ALL fname entries are VAT refs.
 * Hence, any 2 files with the same name are THE SAME FILE in memory.
 * This is why a cp_dir method is not provided, and why the cp_file
 * method does not support maintaining the same filename.
 */

// Length of filename, includes NULL padding
#define FS_NAME_LEN 10

// EOF byte for MFT data/index
#define FS_MFT_END 0x1A


// Typedefs
typedef uint16_t mft_header_t;
typedef uint16_t mft_index_t;
typedef char fs_dat_t;


// Rebuilds MFTi records
void index_fs(void);


// Returns true if MFT is not up to date
bool has_fs_changed(void);


/* Lists a directory's contents (ALWAYS FREE RETURN VALUE AFTER USE)
 *
 * Return codes:
 * <array of fnames and/or dnames> - success
 * 0 - path is empty
 * 1 - path does not exist
 * 255 - not enough memory
 */
fs_dat_t *list_dir_contents(const fs_dat_t *path);


/* Creates an empty directory
 *
 * Return codes:
 * 0 - success
 * 2 - path already exists
 * 3 - parent of path does not exist
 * 4 - invalid directory name
 * 255 - not enough memory
 */
uint8_t mk_dir(const fs_dat_t *path);


/* Removes an empty directory
 *
 * Return codes:
 * 0 - success
 * 1 - path does not exist
 * 5 - path is not empty
 */
uint8_t rm_dir(const fs_dat_t *path);


/* Moves all contents to new directory; deletes original directory
 * If path is empty, equivalent to rm_dir(path) & mk_dir(new_path)
 *
 * Return codes:
 * 0 - success
 * 1 - path does not exist
 * 2 - new_path already exists
 * 3 - parent of new_path does not exist
 * 4 - invalid directory name
 * 255 - not enough memory
 */
uint8_t mv_dir(const fs_dat_t *path, const fs_dat_t *new_path);


/* Creates an empty file at the specified path
 * Return codes:
 * 0 - success
 * 2 - fpath already exists
 * 3 - parent of fpath does not exist
 * 4 - invalid filename
 * 255 - Not enough memory
 */
uint8_t mk_file(const fs_dat_t *fpath);


/* Removes an empty directory
 * Return codes:
 * 0 - success
 * 1 - fpath does not exist
 */
uint8_t rm_file(const fs_dat_t *fpath);


/* Moves file to new file location
 * Return codes:
 * 0 - success
 * 1 - fpath does not exist
 * 2 - new_fpath already exists
 * 3 - parent of new_fpath does not exist
 * 4 - invalid filename
 * 255 - not enough memory
 */
uint8_t mv_file(const fs_dat_t *fpath, const fs_dat_t *new_fpath);


/* Copies a file to new file location
 * Return codes:
 * 0 - success
 * 1 - fpath does not exist
 * 2 - new_fpath already exists
 * 3 - parent of new_fpath does not exist
 * 4 - invalid filename
 * 255 - not enough memory
 */
uint8_t cp_file(const fs_dat_t *fpath, const fs_dat_t *new_fpath);


/* Hardlinks a file to a second location within a directory
 * Return codes:
 * 0 - success
 * 1 - fpath does not exist
 * 2 - hardlink already exists within lnk_path
 * 3 - lnk_path does not exist
 * 255 - not enough memory
 */
uint8_t hl_file(const fs_dat_t *fpath, const fs_dat_t *lnk_path);


#endif