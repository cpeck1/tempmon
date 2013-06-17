#ifndef __INC_FPARSE_H
#define __INC_FPARSE_H

#ifdef __cplusplus
extern "C" {
#endif


/* get index of first occurrence of chr in string; not present returns -1 */
int get_char_index(char *string, char chr);

/*
   get the ID equal to the string phrase in the given line
   returns 1 if id found, and 0 if file is malformed
*/
int get_id_value(char *line, int *id_val);

int get_device_ids(int *device_id, int *vendor_id);

#ifdef __cplusplus
}
#endif

#endif /* __INC_FPARSE_H */
