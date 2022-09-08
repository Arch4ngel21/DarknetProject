#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>


using namespace std;
using namespace cv;
using namespace cv::dnn;


/**
 *	Function draws label on input image for prediction box.
 *
 *	@param input_image - input image, on which label will be drawn
 *	@param label - name of predicted class
 *	@param left - x coordinate of left-top point of prediction box
 *	@param top - y coordinate of left-top point of prediction box
 */ 
void draw_label(Mat& input_image, string label, int left, int top);

/**
 *	Function for computing predictions.
 *
 *	@param input_image - input image based on which making predictions
 *	@param net - opencv dnn model used for output computation
 *
 *	@return vector of output predictions
 */
vector<Mat> pre_process(Mat& input_image, Net& net);

/**
 *	Function filters out predictions and draws boxes to input image.
 *
 *	@param input_image - input image based on which predictions were made. Used for drawing boxes on it.
 *	@param outputs - vector in which predictions are stored
 *	@param class_name - vector of strings, which contains labels for classes
 *
 *	@return image with drawn boxes for valuable predictions.
 */
Mat post_process(Mat& input_image, vector<Mat>& outputs, const vector<string>& class_name);

/**
 *  Main function for making predictions with DarkNet neural network.
 *
 *  @param path - a path to input image (relative or absolute)
 *	@param file - file name (not necessary, because of 'path', but cleaner this way)
 *	@param model - model index based on which was picked in choice box in Application:
 *		0 - Large
 *		1 - Medium
 *		2 - Small
 *		3 - Nano
 *
 *	@return 0 if run was successful, otherwise not 0.
 */
int predict(string path, string file, int model);

