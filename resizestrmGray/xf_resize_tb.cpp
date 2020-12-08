

#include "common/xf_headers.hpp"
#include <stdlib.h>
#include <ap_int.h>
#include "hls_stream.h"


#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "imgproc/xf_resize.hpp"
#include "common/xf_infra.hpp"


#include <ap_axi_sdata.h>

#include "xf_resize_config.h"



template <int TYPE, int ROWS, int COLS, int NPPC>
int AXIstream2xfMat(hls::stream<axis_t> & AXI_video_strm, xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img) {
    axis_t pixelpacket;
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


template <int TYPE, int ROWS, int COLS, int NPPC>
int xfMat2AXIstream(xf::cv::Mat<TYPE, ROWS, COLS, NPPC>& img, hls::stream<axis_t>& AXI_video_strm) {

    axis_t pixelpacket;
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





int main(int argc, char** argv) {
	cv::Mat img, out_img, result_ocv, error;

	    if (argc != 2) {
	        printf("Usage : <executable> <input image> \n");
	        return -1;
	    }

	#if GRAY
	    img.create(cv::Size(WIDTH, HEIGHT), CV_8UC1);
	    out_img.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC1);
	    result_ocv.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC1);
	    error.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC1);
	#else
	    img.create(cv::Size(WIDTH, HEIGHT), CV_8UC3);
	    result_ocv.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC3);
	    out_img.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC3);
	    error.create(cv::Size(NEWWIDTH, NEWHEIGHT), CV_8UC3);
	#endif

	#if GRAY
	    // reading in the color image
	    img = cv::imread(argv[1], 0);
	#else
	    img = cv::imread(argv[1], 1);
	#endif

	    if (!img.data) {
	        return -1;
	    }

	    cv::imwrite("input.png", img);

	    unsigned short in_width, in_height;
	    unsigned short out_width, out_height;

	    in_width = img.cols;
	    in_height = img.rows;
	    out_height = NEWHEIGHT;
	    out_width = NEWWIDTH;

	/*OpenCV resize function*/

	#if INTERPOLATION == 0
	    cv::resize(img, result_ocv, cv::Size(out_width, out_height), 0, 0, CV_INTER_NN);
	#endif
	#if INTERPOLATION == 1
	    cv::resize(img, result_ocv, cv::Size(out_width, out_height), 0, 0, CV_INTER_LINEAR);
	#endif
	#if INTERPOLATION == 2
	    cv::resize(img, result_ocv, cv::Size(out_width, out_height), 0, 0, CV_INTER_AREA);
	#endif


    // Call the top function
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPPC> imgInput(in_height, in_width);
    static xf::cv::Mat<TYPE, NEWHEIGHT, NEWWIDTH, NPPC> imgOutput(out_height, out_width);

    hls::stream<axis_t> srcPtr;
    hls::stream<axis_t> dstPtr;

    imgInput.copyTo(img.data);

    xfMat2AXIstream(imgInput, srcPtr);
    resize_accel(srcPtr, dstPtr, in_height, in_width, out_height,out_width);
    AXIstream2xfMat(dstPtr, imgOutput);

    out_img.data = imgOutput.copyFrom();

    // Write output image
    cv::imwrite("hls_out.jpg", out_img);


    //  Compute absolute difference:
    cv::absdiff(result_ocv, out_img, error);
    cv::imwrite("out_error.jpg", error);
    /*

    // Find minimum and maximum differences.
    double minval = 256, maxval = 0;
    int cnt = 0;
    for (int i = 0; i < in_gray.rows; i++) {
        for (int j = 0; j < in_gray.cols; j++) {
            uchar v = diff.at<uchar>(i, j);
            if (v > 0) cnt++;
            if (minval > v) minval = v;
            if (maxval < v) maxval = v;
        }
    }

    float err_per = 100.0 * (float)cnt / (in_gray.rows * in_gray.cols);

    std::cout << "INFO: Verification results:" << std::endl;
    std::cout << "\tMinimum error in intensity = " << minval << std::endl;
    std::cout << "\tMaximum error in intensity = " << maxval << std::endl;
    std::cout << "\tPercentage of pixels above error threshold = " << err_per << std::endl;

    if (err_per > 0.0f) {
        std::cout << "ERROR: Test Failed." << std::endl;
        //return EXIT_FAILURE;
    }

*/
    return 0;
}

