
/**
  ******************************************************************************
  * @file           : ndef_buffer.h
  * @brief          : NDEF buffer type structures
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/******************************************************************************
 * This file contains code derived from or based on software provided by
 * STMicroelectronics.
 *
 * Original source:
 * STMicroelectronics X-CUBE / BSP / Middleware component
 *
 * Modifications:
 * Copyright (c) 2026 Neon Smart Studio
 * Author: Neon / Neona
 *
 * Licensed under:
 * - Original ST license: ST MIX MYLIBERTY SOFTWARE LICENSE AGREEMENT
 * - Additional modifications may be licensed separately where applicable.
 *
 * The original ST copyright and license notice are preserved below.
 ******************************************************************************/

#ifndef NDEF_BUFFER_H
#define NDEF_BUFFER_H

#include <stdint.h>


/*! NDEF structure to handle const buffers */
typedef struct {
  const uint8_t *buffer; /*!< Pointer to const buffer */
  uint32_t       length; /*!< buffer length           */
} NDef_Const_Buffer;

/*! NDEF structure to handle buffers */
typedef struct {
  uint8_t *buffer; /*!< Pointer to buffer */
  uint32_t length; /*!< buffer length     */
} NDef_Buffer;

/*! NDEF structure to handle const buffers limited to 256 bytes */
typedef struct {
  const uint8_t *buffer; /*!< Pointer to const buffer */
  uint8_t        length; /*!< buffer length           */
} NDef_Const_Buffer_8;

/*! NDEF structure to handle buffers limited to 256 bytes */
typedef struct {
  uint8_t *buffer; /*!< Pointer to buffer */
  uint8_t  length; /*!< buffer length     */
} NDef_Buffer_8;

#endif /* NDEF_BUFFER_H */
