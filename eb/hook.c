/*
 * Copyright (c) 1997, 99, 2000, 01  
 *    Motoyuki Kasahara
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "build-pre.h"
#include "eb.h"
#include "error.h"
#include "appendix.h"
#include "text.h"
#include "build-post.h"

/*
 * Default hookset.
 */
EB_Hookset eb_default_hookset;


/*
 * Initialize default hookset.
 */
void
eb_initialize_default_hookset()
{
    LOG(("in+out: eb_initialize_default_hookset()"));

    eb_initialize_hookset(&eb_default_hookset);
}


/*
 * Initialize a hookset.
 */
void
eb_initialize_hookset(hookset)
    EB_Hookset *hookset;
{
    int i;

    LOG(("in: eb_initialize_hookset()"));

    eb_initialize_lock(&hookset->lock);

    for (i = 0; i < EB_NUMBER_OF_HOOKS; i++) {
	hookset->hooks[i].code = i;
	hookset->hooks[i].function = NULL;
    }
    hookset->hooks[EB_HOOK_STOP_CODE].function
	= eb_hook_stop_code;
    hookset->hooks[EB_HOOK_NARROW_JISX0208].function
	= eb_hook_euc_to_ascii;
    hookset->hooks[EB_HOOK_NARROW_FONT].function
	= eb_hook_narrow_character_text;
    hookset->hooks[EB_HOOK_WIDE_FONT].function
	= eb_hook_wide_character_text;
    hookset->hooks[EB_HOOK_NEWLINE].function
	= eb_hook_newline;

    LOG(("out: eb_initialize_hookset()"));
}


/*
 * Finalize a hookset.
 */
void
eb_finalize_hookset(hookset)
    EB_Hookset *hookset;
{
    int i;

    LOG(("in: eb_finalize_hookset()"));

    for (i = 0; i < EB_NUMBER_OF_HOOKS; i++) {
	hookset->hooks[i].code = i;
	hookset->hooks[i].function = NULL;
    }
    eb_finalize_lock(&hookset->lock);

    LOG(("out: eb_finalize_hookset()"));
}


/*
 * Set a hook.
 */
EB_Error_Code
eb_set_hook(hookset, hook)
    EB_Hookset *hookset;
    const EB_Hook *hook;
{
    EB_Error_Code error_code;

    eb_lock(&hookset->lock);
    LOG(("in: eb_set_hook(hook=%d)", (int)hook->code));

    /*
     * Set a hook.
     */
    if (hook->code < 0 || EB_NUMBER_OF_HOOKS <= hook->code) {
	error_code = EB_ERR_NO_SUCH_HOOK;
	goto failed;
    }
    hookset->hooks[hook->code].function = hook->function;

    LOG(("out: eb_set_hook() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&hookset->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_set_hook() = %s", eb_error_string(error_code)));
    eb_unlock(&hookset->lock);
    return error_code;
}


/*
 * Set a list of hooks.
 */
EB_Error_Code
eb_set_hooks(hookset, hook)
    EB_Hookset *hookset;
    const EB_Hook *hook;
{
    EB_Error_Code error_code;
    const EB_Hook *h;

    eb_lock(&hookset->lock);
    LOG(("in: eb_set_hooks(hooks=[below])"));
#ifdef ENABLE_DEBUG
    for (h = hook; h->code != EB_HOOK_NULL; h++)
	LOG(("    hook=%d", h->code));
#endif

    /*
     * Set hooks.
     */
    for (h = hook; h->code != EB_HOOK_NULL; h++) {
	if (h->code < 0 || EB_NUMBER_OF_HOOKS <= h->code) {
	    error_code = EB_ERR_NO_SUCH_HOOK;
	    goto failed;
	}
	hookset->hooks[h->code].function = h->function;
    }

    /*
     * Unlock the hookset.
     */
    LOG(("out: eb_set_hooks() = %s", eb_error_string(EB_SUCCESS)));
    eb_unlock(&hookset->lock);

    return EB_SUCCESS;

    /*
     * An error occurs...
     */
  failed:
    LOG(("out: eb_set_hooks() = %s", eb_error_string(error_code)));
    eb_unlock(&hookset->lock);
    return error_code;
}


/*
 * EUC JP to ASCII conversion table.
 */
#define EUC_TO_ASCII_TABLE_START	0xa0
#define EUC_TO_ASCII_TABLE_END		0xff

static const unsigned char euc_a1_to_ascii_table[] = {
    0x00, 0x20, 0x00, 0x00, 0x2c, 0x2e, 0x00, 0x3a,     /* 0xa0 */
    0x3b, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x60, 0x00,     /* 0xa8 */
    0x5e, 0x7e, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2f,     /* 0xb8 */
    0x5c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x27,     /* 0xc0 */
    0x00, 0x22, 0x28, 0x29, 0x00, 0x00, 0x5b, 0x5d,     /* 0xc8 */
    0x7b, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd0 */
    0x00, 0x00, 0x00, 0x00, 0x2b, 0x2d, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x3d, 0x00, 0x3c, 0x3e, 0x00, 0x00, 0x00,     /* 0xe0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c,     /* 0xe8 */
    0x24, 0x00, 0x00, 0x25, 0x23, 0x26, 0x2a, 0x40,     /* 0xf0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

static const unsigned char euc_a3_to_ascii_table[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa8 */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,     /* 0xb0 */
    0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb8 */
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,     /* 0xc0 */
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,     /* 0xc8 */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,     /* 0xd0 */
    0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,     /* 0xe0 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,     /* 0xe8 */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,     /* 0xf0 */
    0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};


/*
 * Hook which converts a character from EUC-JP to ASCII.
 */
EB_Error_Code
eb_hook_euc_to_ascii(book, appendix, container, hook_code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    VOID *container;
    EB_Hook_Code hook_code;
    int argc;
    const unsigned int *argv;
{
    int in_code1, in_code2;
    int out_code = 0;

    in_code1 = argv[0] >> 8;
    in_code2 = argv[0] & 0xff;

    if (in_code2 < EUC_TO_ASCII_TABLE_START
	|| EUC_TO_ASCII_TABLE_END < in_code2) {
	out_code = 0;
    } else if (in_code1 == 0xa1) {
	out_code = euc_a1_to_ascii_table[in_code2 - EUC_TO_ASCII_TABLE_START];
    } else if (in_code1 == 0xa3) {
	out_code = euc_a3_to_ascii_table[in_code2 - EUC_TO_ASCII_TABLE_START];
    }

    if (out_code == 0)
	eb_write_text_byte2(book, in_code1, in_code2);
    else
	eb_write_text_byte1(book, out_code);

    return EB_SUCCESS;
}


/*
 * Hook for stop-code.
 */
EB_Error_Code
eb_hook_stop_code(book, appendix, container, hook_code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    VOID *container;
    EB_Hook_Code hook_code;
    int argc;
    const unsigned int *argv;
{
    EB_Error_Code error_code = EB_SUCCESS;

    if (appendix == NULL
	|| appendix->subbook_current == NULL
	|| appendix->subbook_current->stop0 == 0) {
	if (argv[0] == 0x1f41 && argv[1] == book->text_context.auto_stop_code)
	    error_code = EB_ERR_STOP_CODE;
    } else {
	if (argv[0] == appendix->subbook_current->stop0
	    && argv[1] == appendix->subbook_current->stop1)
	    error_code = EB_ERR_STOP_CODE;
    }

    return error_code;
}


/*
 * Hook for narrow local character.
 */
EB_Error_Code
eb_hook_narrow_character_text(book, appendix, container, hook_code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    VOID *container;
    EB_Hook_Code hook_code;
    int argc;
    const unsigned int *argv;
{
    char alt_text[EB_MAX_ALTERNATION_TEXT_LENGTH + 1];

    if (appendix == NULL
	|| eb_narrow_alt_character_text(appendix, argv[0], alt_text)
	!= EB_SUCCESS) {
	eb_write_text_string(book, "<?>");
    } else {
	eb_write_text_string(book, alt_text);
    }

    return EB_SUCCESS;
}


/*
 * Hook for wide local character.
 */
EB_Error_Code
eb_hook_wide_character_text(book, appendix, container, hook_code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    VOID *container;
    EB_Hook_Code hook_code;
    int argc;
    const unsigned int *argv;
{
    char alt_text[EB_MAX_ALTERNATION_TEXT_LENGTH + 1];

    if (appendix == NULL
	|| eb_wide_alt_character_text(appendix, argv[0], alt_text)
	!= EB_SUCCESS) {
	eb_write_text_string(book, "<?>");
    } else {
	eb_write_text_string(book, alt_text);
    }

    return EB_SUCCESS;
}


/*
 * Hook for a newline character.
 */
EB_Error_Code
eb_hook_newline(book, appendix, container, code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    void *container;
    EB_Hook_Code code;
    int argc;
    const unsigned int *argv;
{
    eb_write_text_byte1(book, '\n');

    return 0;
}


/*
 * Hook which does nothing.
 */
EB_Error_Code
eb_hook_empty(book, appendix, container, hook_code, argc, argv)
    EB_Book *book;
    EB_Appendix *appendix;
    VOID *container;
    EB_Hook_Code hook_code;
    int argc;
    const unsigned int *argv;
{
    return EB_SUCCESS;
}


