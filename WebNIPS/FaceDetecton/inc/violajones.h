#ifndef VIOLA_JONES_H
#define VIOLA_JONES_H

#include <vector>

class Rect
{
public:
    Rect():x(0),y(0),width(0),height(0) {}
    Rect(int oX, int oY, int w, int h):x(oX), y(oY), width(w), height(h) {}

    // Setters
    void set(int x, int y, int width, int height);
    void setX(int x){this->x = x;}
    void setY(int y){this->y = y;}
    void setWidth(int w){this->width = w;}
    void setHeight(int h) {this->height = h;}

    //Getters
    int getX(){return x;}
    int getY(){return y;}
    int getWidth(){return width;}
    int getHeight(){return height;}

    // Area
    double area(){return width*height;}

    // Shift origin
    void shift(int dX, int dY);

    // Scale referencing coordinate system origin
    void scale(double sX, double sY);

    // Scale referencing rectangle origin
    void origScale(double sX, double sY);

    // Scale referencing recntangle center
    void centerScale(double sX, double sY);

    // Check if center is inside another rect
    bool isInside(Rect&);

    // Constraint to rectangle
    void limitTo(Rect&);

private:
    int x;                      // X origin
    int y;                      // Y origin
    int width;                  // Width
    int height;                 // Height
};



// Split string on delimiter. Return tokens
std::vector<double> split(std::string str, char delimiter);

class Frame
{
public:
    // Frame constructor
    Frame(float* dt, float* sqDt, unsigned int w, unsigned int h): data(dt), sqData(sqDt), width(w), height(h) {}

    //Getters
    unsigned int getWidth() {return width;}
    unsigned int getHeight() {return height;}
    float* getData() {return data;}
    float* getsqData() {return sqData;}

    // Access data
    double get(unsigned int x, unsigned int y);

    // Access squared data
    double getSq(unsigned int x, unsigned int y);

    // Get sum over Rect
    double sumOver(Rect&);

    // Extract mean, divide by standard deviation and get sum over Rect
    double sumOver(Rect&, double, double);

    // Get sum over squared Rect
    double sumOverSq(Rect&);

    // Extract mean and get sum over squared Rect
    double sumOverSq(Rect&, double);

    // Compute standard deviation
    double stdDevOver(Rect&);

private:
    float* data;    // Pointer to data
    float* sqData;  // Pointer to squared data
    unsigned int width;     // Frame width
    unsigned int height;    // Frame height
};

class Feature
{
public:
    std::vector<std::pair<Rect,double> > rect;     // Feature geometry
    double T;                                      // Threshold
    double lVal;                                   // Value to accumulate if <T
    double rVal;                                   // Value to accumulate if >T
    std::pair<unsigned int, unsigned int> refSize; // Reference window size (pixels)

    // Constructor
    Feature(std::vector<std::pair<Rect,double> >& r, double t, double lV, double rV, std::pair<unsigned int, unsigned int> rfSz):
            rect(r), T(t), lVal(lV), rVal(rV), refSize(rfSz) {}

    // Extract feature from frame
    double extract(Rect&, Frame&, double mean, double stdDev);
};

class Stage
{
public:
    std::vector<Feature> feat;  // Collection of features
    double T;                   // Threshold

    // Stage constructor
    Stage(std::vector<Feature> f, double t): feat(f), T(t) {}

    // Apply stage to frame
    std::vector<Rect> apply(std::vector<Rect>, Frame&);
};

class Cascade
{
public:
    unsigned int sizeH;         // Reference height (pixels)
    unsigned int sizeW;         // Reference width (pixels)
    std::vector<Stage> layer;

    //Cascade default constructor
    Cascade():sizeH(0),sizeW(0),layer(){}

    // Constructor from file
    Cascade(std::string& file);

    // Constructor from specific data array
    Cascade(const int W, const int H, double haar1[2912][20]);
};

class Detector
{
public:
    const double scale;         // Control scaling during application (1: no scaling)
    const double shift;         // Control shifting during application (in %)
    const unsigned int wSize;   // Reference square window size during application (pixels)

    Detector(double sc, double sh, const unsigned int sz): scale(sc), shift(sh), wSize(sz){}

    // Apply cascaded detector to image
    Rect apply(Cascade&, Rect&, Frame&);

    // Apply detector to set of windows in image
    std::vector<Rect> step(Cascade&, std::vector<Rect>, Frame&);

    // Generate set of windows in ROI
    std::vector<Rect> generateWindows(Rect& roi, Rect& win);
};

// Merge overlapped windows
class Merger
{
public:
    // Constructor
    Merger(double c): confidence(c){}

    // Apply merging to set of windows according to criteria
    std::pair<Rect,double> apply(Frame&, std::vector<Rect>&, unsigned int);

    // Get cluster representatnt as mean of all members of the cluster
    std::pair<int, int> getClusterRepresentant(std::vector<std::pair<int, int> >& in);

private:
    double confidence;               // Merger parameter
};

class ViolaJones
{
float* integralImage;
float* sqIntegralImage;
Cascade cascade;
int    imW;
int    imH;
static const int nChn = 4;

public:
    ViolaJones(int w, int h);
    ~ViolaJones();
    Rect detect(unsigned char* image, unsigned short* rect);
    Rect track(unsigned char* image, unsigned short* rect, unsigned short* roi);
    int getW() {return imW;}
    int getH() {return imH;}

private:
    void generateIntegralImage(unsigned char* image);
    void generateSquareIntegralImage(unsigned char* image);
};

#endif
