/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XF_EROSION_CONFIG_H_
#define _XF_EROSION_CONFIG_H_

#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "imgproc/xf_resize.hpp"
//#include "xf_config_params.h"
#include "common/xf_infra.hpp"

#include <ap_axi_sdata.h>


#define RO 0 // Resource Optimized (8-pixel implementation)
#define NO 1 // Normal Operation (1-pixel implementation)

// For Nearest Neighbor & Bilinear Interpolation, max down scale factor 2 for all 1-pixel modes, and for upscale in x
// direction
#define MAXDOWNSCALE 4

#define RGB 0
#define GRAY 1
/* Interpolation type*/
#define INTERPOLATION 2
// 0 - Nearest Neighbor Interpolation
// 1 - Bilinear Interpolation
// 2 - AREA Interpolation

/* Input image Dimensions */
//#define WIDTH 128  // Maximum Input image width
//#define HEIGHT 128 // Maximum Input image height

/* Output image Dimensions */
//#define NEWWIDTH 64  // Maximum output image width
//#define NEWHEIGHT 64 // Maximum output image height


/* config width and height */
#define WIDTH 640
#define HEIGHT 480

#define NEWWIDTH 320
#define NEWHEIGHT 240

/*  define the input and output types  */
#if NO
#define NPC_T XF_NPPC1
#else // RO
#define NPC_T XF_NPPC8
#endif

#if GRAY
#define TYPE XF_8UC1
#define CH_TYPE XF_GRAY
#else // RGB
#define TYPE XF_8UC3
#define CH_TYPE XF_RGB
#endif


// Resolve optimization type:
#if NO
#define NPC1 XF_NPPC1
#if GRAY
#define PTR_WIDTH 8
#else
#define PTR_WIDTH 24
#endif
#endif

#define NPPC XF_NPPC1

//#if RO
//#define NPC1 XF_NPPC8
//#if GRAY
//#define PTR_WIDTH 64
//#else
//#define PTR_WIDTH 256
//#endif
//#endif
// Set pixel depth:
#if GRAY
#define TYPE XF_8UC1
#else
#define TYPE XF_8UC3
#endif




/* Define image format */
#define RGBA_TYPE XF_8UC4
#define SINGLE_COLOR_TYPE XF_8UC1
//const int WIDTH_MAX = 2048;
//const int HEIGHT_MAX = 1536;

/* Define the AXI Stream type */
const int bytes_per_pixel = 1;

struct axis_t {
    ap_uint<8> data;
    ap_int<1> last;
};

typedef hls::stream<axis_t> stream_t;


//////////////////////////////////////////////////////////////////
#define RO 0 // Resource Optimized (8-pixel implementation)
#define NO 1 // Normal Operation (1-pixel implementation)


/////////////////////////////////////////////////////////////////////


/* config width and height */

/* Accelerator specific parameters */

template <int TYPE, int ROWS, int COLS, int NPPC>
int xfMat2AXIstream(xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img, hls::stream<axis_t>& AXI_video_strm);

template <int TYPE, int ROWS, int COLS, int NPPC>
int AXIstream2xfMat(hls::stream<axis_t> & AXI_video_strm, xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img);

void resize_accel(stream_t& stream_in, stream_t& stream_out, int rows_in, int cols_in,  int rows_out,  int cols_out);

#endif // _XF_EROSION_CONFIG_H_
