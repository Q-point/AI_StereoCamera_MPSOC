#include "xf_resize_config.h"


/*
Unpack a AXI video stream into a xf::cv::Mat<> object
 *input: AXI_video_strm
 *output: img
 */
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

// Pack the data of a xf::cv::Mat<> object into an AXI Video stream
/*
 *  input: img
 *  output: AXI_video_strm
 */
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

void resize_accel(stream_t& stream_in, stream_t& stream_out, int rows_in, int cols_in,  int rows_out,  int cols_out) {

    #pragma HLS INTERFACE s_axilite port=rows_in              
    #pragma HLS INTERFACE s_axilite port=cols_in              
    #pragma HLS INTERFACE s_axilite port=rows_out              
    #pragma HLS INTERFACE s_axilite port=cols_out              
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE axis port=stream_in
    #pragma HLS INTERFACE axis port=stream_out

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPPC> in_mat(rows_in, cols_in);
    xf::cv::Mat<TYPE, NEWHEIGHT, NEWWIDTH, NPPC> out_mat(rows_out, cols_out);


     // clang-format off
     #pragma HLS DATAFLOW
     // clang-format on

     AXIstream2xfMat<TYPE,HEIGHT,WIDTH,NPPC>(stream_in, in_mat);
     xf::cv::resize<INTERPOLATION, TYPE, HEIGHT, WIDTH, NEWHEIGHT, NEWWIDTH, NPPC, MAXDOWNSCALE>(in_mat, out_mat);
     xfMat2AXIstream<TYPE,NEWHEIGHT,NEWWIDTH,NPPC>(out_mat, stream_out);

}


