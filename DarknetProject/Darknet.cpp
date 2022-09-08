#include "Darknet.h"

const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.5;
const float NMS_THRESHOLD = 0.45;
const float CONFIDENCE_THRESHOLD = 0.45;

const float FONT_SCALE = 0.9;
const int FONT_FACE = FONT_HERSHEY_SIMPLEX;
const int THICKNESS = 2;

Scalar BLACK = Scalar(0, 0, 0);
Scalar BLUE = Scalar(155, 178, 50);
Scalar YELLOW = Scalar(0, 255, 255);
Scalar RED = Scalar(0, 0, 255);


void draw_label(Mat& input_image, string label, int left, int top)
{
    int baseLine;
    Size label_size = getTextSize(label, FONT_FACE, FONT_SCALE, THICKNESS, &baseLine);

    top = max(top, label_size.height);

    Point tlc = Point(left, top);

    Point brc = Point(left + label_size.width, top + label_size.height + baseLine);

    rectangle(input_image, tlc, brc, BLACK, FILLED);
    putText(input_image, label, Point(left, top + label_size.height), FONT_FACE, FONT_SCALE, YELLOW, THICKNESS);
}

vector<Mat> pre_process(Mat& input_image, Net& net)
{
    // Image to blob
    Mat blob;
    blobFromImage(input_image, blob, 1. / 255., Size(INPUT_WIDTH, INPUT_HEIGHT), Scalar(), true, false);

    net.setInput(blob);

    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());       // forward pass

    return outputs;
}

Mat post_process(Mat& input_image, vector<Mat>& outputs, const vector<string>& class_name)
{
    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;

    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;
    float* data = (float*)outputs[0].data;
    const int dimensions = 85;          // X; Y; W; H; Confidence; 80 Class scores - vector dimension = 85
    const int rows = 25200;             // 25200 for size 640

    for (int i = 0; i < rows; ++i)
    {
        float confidence = data[4];

        if (confidence >= CONFIDENCE_THRESHOLD)
        {
            float* classes_scores = data + 5;
            Mat scores(1, class_name.size(), CV_32FC1, classes_scores);
            Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

            if (max_class_score > SCORE_THRESHOLD)
            {
                confidences.push_back(confidence);
                class_ids.push_back(class_id.x);
                float cx = data[0];
                float cy = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((cx - 0.5 * w) * x_factor);
                int top = int((cy - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);

                boxes.push_back(Rect(left, top, width, height));
            }
        }

        data += 85;
    }

    vector<int> indices;
    NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices);
    for (int i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];

        int left = box.x;
        int top = box.y;
        int width = box.width;
        int height = box.height;

        rectangle(input_image, Point(left, top), Point(left + width, top + height), BLUE, 3 * THICKNESS);
        string label = format("%.2f", confidences[idx]);
        label = class_name[class_ids[idx]] + ":" + label;
        draw_label(input_image, label, left, top);
    }
    return input_image;
}


int predict(string path, string file, int model)
{
    // ----------------- Loading class labels -----------------
    vector<string> class_list;
    ifstream ifs("coco.names");
    string line;
    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }

    // --------------------- Loading image ---------------------
    Mat frame;
    frame = imread(path);

    // -------------------- Loading network --------------------
    Net net;

    switch (model)
    {
		case 0:
            net = readNet("models/yolov5l.onnx");
			break;
		case 1:
            net = readNet("models/yolov5m.onnx");
            break;
		case 2:
			net = readNet("models/yolov5s.onnx");
            break;
		case 3:
			net = readNet("models/yolov5n.onnx");
			break;
        default:
            cout << "Wrong model index!" << endl;
            net = readNet("models/yolov5l.onnx");
            break;
    }


    // ----------- Making detections using network -----------
    vector<Mat> detections;
    detections = pre_process(frame, net);

    // --------- Printing predictions to input image ---------
    Mat cloned_frame = frame.clone();
    Mat img = post_process(cloned_frame, detections, class_list);

    vector<double> layersTimes;
    double freq = getTickFrequency() / 1000;
    double t = net.getPerfProfile(layersTimes) / freq;
    string label = format("Inference time : %.2f ms", t);
    putText(img, label, Point(20, 40), FONT_FACE, FONT_SCALE, RED);

    string res_file = "temp/" + file;
    imwrite(res_file, img);

    return 0;
}

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie