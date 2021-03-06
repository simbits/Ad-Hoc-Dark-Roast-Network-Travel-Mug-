/**
 * TagFactory.h
 * (c) 2004 - 2006 SkyeTek, Inc. All Rights Reserved.
 *
 * Creates reader objects.
 */
#ifndef TAG_FACTORY_H
#define TAG_FACTORY_H

#include "../SkyeTekAPI.h"
#include "Tag.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a tag for the given tag type.
 */
SKYETEK_STATUS CreateTagImpl(SKYETEK_TAGTYPE type, LPSKYETEK_ID lpId, LPSKYETEK_TAG* tag);

/**
 * Initializes the tag type for the given tag.
 */
SKYETEK_STATUS InitializeTagType(LPSKYETEK_TAG tag);

/**
 * Frees the tag.
 * @param tag Tag to free
 */
void FreeTagImpl(LPSKYETEK_TAG tag);

/**
 * Duplicate the tag.
 * @param tag Tag to free
 */
LPSKYETEK_TAG DuplicateTagImpl(LPSKYETEK_TAG tag);

#ifdef __cplusplus
}
#endif

#endif 



