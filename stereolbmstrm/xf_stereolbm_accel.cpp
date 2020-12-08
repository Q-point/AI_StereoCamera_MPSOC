#include "xf_stereolbm_config.h"
#include "core/xf_convert_bitdepth.hpp"

/*
Unpack a AXI video stream into a xf::cv::Mat<> object
 *input: AXI_video_strm
 *output: img
 */

template <int TYPE, int ROWS, int COLS, int NPPC>
int AXIstream2xfMat(hls::stream<axis_t<8>>& AXI_video_strm, xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img) {
    axis_t<8> pixelpacket;
    int res = 0;

    int rows = img.rows;
    int cols = img.cols;
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

	loop_row_axi2mat:   for (int i = 0; i < rows; i++) {
		loop_col_axi2mat: 	for (int j = 0; j < cols; j++) {
			// clang-format off
	    		#pragma HLS loop_flatten off
	    		#pragma HLS pipeline II=1
			// clang-format on
			AXI_video_strm >> pixelpacket;
		        img.write(idx++, pixelpacket.data);
		}
	   }
    return res;
}

// Pack the data of a xf::cv::Mat<> object into an AXI Video stream
/*
 *  input: img
 *  output: AXI_video_strm
 */
template <int TYPE, int ROWS, int COLS, int NPPC>
int xfMat2AXIstream(xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img, hls::stream<axis_t<8>>& AXI_video_strm) {

    axis_t<8> pixelpacket;
    int res = 0;

    int rows = img.rows;
    int cols = img.cols;
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

    bool sof = true; // Indicates start of frame

	loop_row_mat2axi: for (int i = 0; i < rows; i++) {
		loop_col_mat2axi: for (int j = 0; j < cols; j++) {
			// clang-format off
			#pragma HLS loop_flatten off
			#pragma HLS pipeline II=1
			// clang-format on
			    
				ap_uint<1> tmp = 0;
				if ((i==rows-1) && (j== cols-1)) {
					tmp = 1;
				}

				pixelpacket.last = tmp;
				pixelpacket.data = img.read(idx++);

				AXI_video_strm << pixelpacket;

			}
		}

    return res;
}

/******************************************************************************************************************/

/*
Unpack a AXI video stream into a xf::cv::Mat<> object
 *input: AXI_video_strm
 *output: img
 */

template <int TYPE, int ROWS, int COLS, int NPPC>
int AXIstreamwide2xfMat(streamwide_t& AXI_video_strm, xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img) {
    axis_t<16> pixelpacket;
    int res = 0;

    int rows = img.rows;
    int cols = img.cols;
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

	loop_row_axi2mat:   for (int i = 0; i < rows; i++) {
		loop_col_axi2mat: 	for (int j = 0; j < cols; j++) {
			// clang-format off
	    		#pragma HLS loop_flatten off
	    		#pragma HLS pipeline II=1
			// clang-format on
			AXI_video_strm >> pixelpacket;
		        img.write(idx++, pixelpacket.data);
		}
	   }
    return res;
}


// Pack the data of a xf::cv::Mat<> object into an AXI Video stream
/*
 *  input: img
 *  output: AXI_video_strm
 */
template <int TYPE, int ROWS, int COLS, int NPPC>
int xfMat2AXIstreamwide(xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img, streamwide_t& AXI_video_strm) {

    axis_t<16> pixelpacket;
    int res = 0;

    int rows = img.rows;
    int cols = img.cols;
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

    bool sof = true; // Indicates start of frame

	loop_row_mat2axi: for (int i = 0; i < rows; i++) {
		loop_col_mat2axi: for (int j = 0; j < cols; j++) {
			// clang-format off
			#pragma HLS loop_flatten off
			#pragma HLS pipeline II=1
			// clang-format on

				ap_uint<1> tmp = 0;
				if ((i==rows-1) && (j== cols-1)) {
					tmp = 1;
				}

				pixelpacket.last = tmp;
				pixelpacket.data = img.read(idx++);

				AXI_video_strm << pixelpacket;

			}
		}

    return res;
}


#define _TEXTURE_THRESHOLD_ 20
#define _UNIQUENESS_RATIO_ 15
#define _PRE_FILTER_CAP_ 31
#define _MIN_DISP_ 0


void stereolbm_accel(stream_t& stream_inL,stream_t& stream_inR, streamwide_t& stream_out, int height, int width) {
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE axis port=stream_inL
    #pragma HLS INTERFACE axis port=stream_inR
    #pragma HLS INTERFACE axis port=stream_out

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPCC> imgInputL(height, width);
    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPCC> imgInputR(height, width);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPCC> imgOutput(height, width);
   
    //xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPCC> imgOutputStream(height, width);

    xf::cv::xFSBMState<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS> bmState;

    // Initialize SBM State:
   /*
    bmState.preFilterCap = bm_state_in[0];
    bmState.uniquenessRatio = bm_state_in[1];
    bmState.textureThreshold = bm_state_in[2];
    bmState.minDisparity = bm_state_in[3];
*/

    bmState.preFilterCap = 31;
    bmState.uniquenessRatio = 15;
    bmState.textureThreshold = 20;
    bmState.minDisparity =  0;

 
// clang-format off
	#pragma HLS DATAFLOW
    // clang-format on

    // Retrieve xf::Mat objects from img_in data:
    AXIstream2xfMat<IN_TYPE,HEIGHT,WIDTH,NPCC>(stream_inL, imgInputL);
    AXIstream2xfMat<IN_TYPE,HEIGHT,WIDTH,NPCC>(stream_inR, imgInputR);

    // Run xfOpenCV kernel:
    xf::cv::StereoBM<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS, IN_TYPE, OUT_TYPE, HEIGHT, WIDTH, NPCC,XF_USE_URAM>(imgInputL, imgInputR, imgOutput, bmState);

    // Convert _dst xf::Mat object to output array:
    xfMat2AXIstreamwide<OUT_TYPE,HEIGHT,WIDTH,NPCC>(imgOutput, stream_out);
    
}

 // End of kernel
