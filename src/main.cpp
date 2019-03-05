#include <iostream>
#include <opencv2/opencv.hpp>
#include "mtcnn.h"

using namespace std;
using namespace cv;
using namespace face;

#ifdef _MSC_VER
#include <intrin.h>
string cpu_info()
{
  int id[4] = { -1 };
  char info[32] = { 0 };
  string cpu_info;
  for (uint i = 0; i < 3; i++) {
    __cpuid(id, 0x80000002 + i);
    memcpy(info, id, sizeof(id));
    cpu_info += info;
  }
  return cpu_info;
}
#else
#include <fstream>
string cpu_info()
{
  ifstream file("/proc/cpuinfo");
  string line;
  for (int i = 0; i < 5; i++)
    getline(file, line);
  return line.substr(13);
}
#endif

Mat imdraw(const Mat im, const vector<BBox> & bboxes)
{
  Mat canvas = im.clone();
  for (const auto & bbox : bboxes) {
    rectangle(canvas, Rect(bbox.x1, bbox.y1, bbox.x2-bbox.x1, bbox.y2-bbox.y1), Scalar(255, 0, 0), 2);
    for (int i = 0; i < 5; i++) {
      circle(canvas, Point((int)bbox.fpoints[i], (int)bbox.fpoints[i+5]), 5, Scalar(0, 255, 0), -1);
    }
    putText(canvas, to_string(bbox.score), Point(bbox.x1, bbox.y1),
      FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2, LINE_AA);
  }
  return canvas;
}

void demo() {
  Mtcnn mtcnn("../models");
  Mat image = imread("../sample.jpg");
  vector<BBox> bboxes = mtcnn.Detect(image);
  Mat canvas = imdraw(image, bboxes);
  imshow("mtcnn face detector", canvas);
  cv::waitKey(0);
}

void performance(bool lnet = true, int ntimes = 50) {
  Mtcnn mtcnn("../models", lnet);
  Mat image = imread("../sample.jpg");
  clock_t begin = clock();
  for (int i = 0; i < ntimes; i++)
    mtcnn.Detect(image);
  clock_t end = clock();
  string disc_head = lnet ? "With LNet" : "Without LNet";
  string disc_pad = "=============";
  cout << disc_pad << " " << disc_head << " " << disc_pad << endl;
  cout << "cpu info: " << cpu_info() << endl;
  cout << "image shape: (" << image.cols << ", " << image.rows << ", " << image.channels() << ")" << endl;
  cout << "performance : " << CLOCKS_PER_SEC / (double)(end - begin) * ntimes << " fps" << endl;
  cout << "detect time: " << (double)(end - begin) / ntimes << " ms" << endl;
}

#include <fstream>
void fddb_detect(const string name = "mtcnn") {
  Mtcnn mtcnn("../models", false);
  mtcnn.thresholds[0] = 0.6;
  mtcnn.thresholds[1] = 0.7;
  mtcnn.thresholds[2] = 0.7;
  mtcnn.face_min_size = 20;
  mtcnn.face_max_size = 2000;
  string fddb_root = "D:/vcodes/win-ncnn/FDDB";
  string img_root = fddb_root + "/images/";
  ifstream img_list(fddb_root + "/imList.txt");
  ofstream dets(fddb_root + "/dets/" + name + ".txt");
  string line;
  int cnt = 0;
  while (getline(img_list, line)) {
    cnt += 1;
    cout << cnt << ": " << line << endl;
    Mat image = imread(img_root + line + ".jpg");
    vector<BBox> bboxes = mtcnn.Detect(image);
    dets << line << endl;
    dets << bboxes.size() << endl;
    for (auto & bbox : bboxes) {
      dets << bbox.x1 << " " << bbox.y1 << " ";
      dets << bbox.x2 - bbox.x1 << " " << bbox.y2 - bbox.y1 << " ";
      dets << bbox.score << endl;
    }
  }
}

int main() {
  performance(true);
  performance(false);
  //fddb_detect("cvdnn");
  demo();
  return 0;
}
