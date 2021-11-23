//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
//
//  September, 2020
//
//  ------------------------
//  FakeGL.cpp
//  ------------------------
//  
//  A unit for implementing OpenGL workalike calls
//  
///////////////////////////////////////////////////

#include "FakeGL.h"
#include <math.h>

#include "Cartesian3.h"
#include "Homogeneous4.h"
#include "Matrix4.h"
#include "RGBAImage.h"
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

//-------------------------------------------------//
//                                                 //
// CONSTRUCTOR / DESTRUCTOR                        //
//                                                 //
//-------------------------------------------------//

// constructor
FakeGL::FakeGL()
    { // constructor
        //in constructor function, we only need to initialize the properties belonging to this class.
        //for some values that is worthy to explain, I will do that.
        this->viewPortWidth = 0;
        this->viewPortHeight = 0;

        this->attributeColour.red = this->attributeColour.green = this->attributeColour.blue = this->attributeColour.alpha = 1.0;
        //the depthColor is used to store value in depthbuffer, as depthbuffer needs this type of data.
        //in depthColour only alpha is represented as the value of depth, so we set other values as 0,
        //but set alpha as 255, as any value less than 255 will be stored in depthbuffer
        this->depthColour.red =  this->depthColour.green =  this->depthColour.blue = 0;
        this->depthColour.alpha = 255;

        this->attributeNormal.x = 0;
        this->attributeNormal.y = 0;
        this->attributeNormal.z = 1;// just avoid attributeNormal to be zero vector

        Matrix4 identityMatrix = Matrix4();
        identityMatrix.SetIdentity();
        //pushing an identity matrix in is safe
        this->stackModelView.push_back(identityMatrix);
        this->stackProjection.push_back(identityMatrix);

        this->isLighting = false;
        this->isTexture = false;
        this->isDepthTest = false;
        this->isPhongShading = true;

        this->lightPosition = Homogeneous4(0,0,1,0); //avoid zero vector and initialize it as a directional light
        this->ambientLight[0] = this->ambientLight[1] = this->ambientLight[2] = 0;
        this->ambientLight[3] = 1;
        this->diffuseLight[0] = this->diffuseLight[1] = this->diffuseLight[2] = 0;
        this->diffuseLight[3] = 1;
        this->specularLight[0] = this->specularLight[1] = this->specularLight[2] = 0;
        this->specularLight[3] = 1;

        this->exponent = 0;

        this->ambientMaterial[0] = this->ambientMaterial[1] = this->ambientMaterial[2] = 0.1;
        this->ambientMaterial[3] = 1;
        this->diffuseMaterial[0] = this->diffuseMaterial[1] = this->diffuseMaterial[2] = 0.9;
        this->diffuseMaterial[3] = 1;
        this->specularMaterial[0] = this->specularMaterial[1] = this->specularMaterial[2] = 0;
        this->specularMaterial[3] = 1;
        this->emissiveMaterial[0] = this->emissiveMaterial[1] = this->emissiveMaterial[2] = 0;
        this->emissiveMaterial[3] = 1;

        this->primitiveType = -1;
        this->pointSize = 1;
        this->lineWidth = 1;

        this->attributeU = 0;
        this->attributeV = 0;

        this->near = 1;
        this->far = -1;


    } // constructor

// destructor
FakeGL::~FakeGL()
    { // destructor
    } // destructor

//-------------------------------------------------//
//                                                 //
// GEOMETRIC PRIMITIVE ROUTINES                    //
//                                                 //
//-------------------------------------------------//

// starts a sequence of geometric primitives
void FakeGL::Begin(unsigned int PrimitiveType)
    { // Begin()
        //from the name "Begin", we can know that this function is a start point of some process,
        //so store the PrimitiveType.
        //and we also can understan the function's role by checking where it is called.
        this->primitiveType = PrimitiveType;
    } // Begin()

// ends a sequence of geometric primitives
void FakeGL::End()
    { // End()
        this->primitiveType = -1; //set it as -1 is safe. and a good habit for programming
    } // End()

// sets the size of a point for drawing
void FakeGL::PointSize(float size)
    { // PointSize()
        //here we need to guarantee pointSize is integer, as in pixels, we can not use decimal value.
        //and at least is 1.
        this->pointSize = round(size);
        if (this->pointSize < 1)
            this->pointSize = 1;
    } // PointSize()

// sets the width of a line for drawing purposes
void FakeGL::LineWidth(float width)
    { // LineWidth()
        //here we need to guarantee lineWidth is integer, as in pixels, we can not use decimal value.
        //and at least is 1.
        this->lineWidth = round(width);
        if (this->lineWidth < 1)
            this->lineWidth = 1;

    } // LineWidth()

//-------------------------------------------------//
//                                                 //
// MATRIX MANIPULATION ROUTINES                    //
//                                                 //
//-------------------------------------------------//

// set the matrix mode (i.e. which one we change)   
void FakeGL::MatrixMode(unsigned int whichMatrix)
    { // MatrixMode()
        //this function is important, when a state is set via this function,we need to store it.
        //We will use the state to determine which matrix stack can be used.
        this->matrixState = whichMatrix;
    } // MatrixMode()

// pushes a matrix on the stack
void FakeGL::PushMatrix()
    { // PushMatrix()
        //when we need to use the current matrix in the stacks, and the calculation can affect the current matrix,
        //we need to store a copy of current matrix, so we push as copy of the last one in the stacks.
        //when we set the light we need to do this.
        if (this->matrixState & FAKEGL_MODELVIEW){
            this->stackModelView.push_back(this->stackModelView.back());
        } else if (this->matrixState & FAKEGL_PROJECTION){
            this->stackProjection.push_back(this->stackProjection.back());
        }
    } // PushMatrix()

// pops a matrix off the stack
void FakeGL::PopMatrix()
    { // PopMatrix()
        //when we do not need the last matrix in the stacks we need to pop it out.
        if (this->matrixState & FAKEGL_MODELVIEW) {
            this->stackModelView.pop_back();
        } else if (this->matrixState & FAKEGL_PROJECTION){
            this->stackProjection.pop_back();
        }

    } // PopMatrix()

// load the identity matrix
void FakeGL::LoadIdentity()
    { // LoadIdentity()
        //this function just push an identity in current stack
        if (this->matrixState & FAKEGL_MODELVIEW) {
            this->stackModelView.back().SetIdentity();
        }
        if (this->matrixState & FAKEGL_PROJECTION) {
            this->stackProjection.back().SetIdentity();
        }

    } // LoadIdentity()

// multiply by a known matrix in column-major format
void FakeGL::MultMatrixf(const float *columnMajorCoordinates)
    { // MultMatrixf()
    //this function is easy, just math
        Matrix4 currentBackMatrix;
        Matrix4 resultMatrix = Matrix4();
        resultMatrix.SetZero();
        if (this->matrixState & FAKEGL_MODELVIEW) {
            currentBackMatrix = this->stackModelView.back();
            this->stackModelView.pop_back();
            for (int r = 0; r < 4; ++r) {
                for (int column = 0; column < 4; ++column) {
                    for (int item = 0; item < 4; ++item) {
                        resultMatrix.coordinates[r][column] +=
                                currentBackMatrix.coordinates[r][item] * columnMajorCoordinates[4 * column + item];
                    }
                }//for (int column = 0; column < 4; ++column)
            }//for (int r = 0; r < 4; ++r)
            this->stackModelView.push_back(resultMatrix);
        }
        if (this->matrixState & FAKEGL_PROJECTION){
            currentBackMatrix = this->stackProjection.back();
            this->stackProjection.pop_back();
            for (int r = 0; r < 4; ++r) {
                for (int column = 0; column < 4; ++column) {
                    for (int item = 0; item < 4; ++item) {
                        resultMatrix.coordinates[r][column] += currentBackMatrix.coordinates[r][item] * columnMajorCoordinates[4*column + item];
                    }
                }//for (int column = 0; column < 4; ++column)
            }//for (int r = 0; r < 4; ++r)
            this->stackProjection.push_back(resultMatrix);
        }
    } // MultMatrixf()

// sets up a perspective projection matrix
void FakeGL::Frustum(float left, float right, float bottom, float top, float zNear, float zFar)
    { // Frustum()
    //use the formula from our ppt
        if(left == right || bottom == top || zNear == zFar)
            return;
        Matrix4 frustumMatrix = Matrix4();
        frustumMatrix.SetZero();
        frustumMatrix[0][0] = 2*zNear/(right-left);
        frustumMatrix[0][2] = (right + left)/(right - left);
        frustumMatrix[1][1] = 2*zNear/(top-bottom);
        frustumMatrix[1][2] = (top + bottom)/(top - bottom);
        frustumMatrix[2][2] = -1*(zFar + zNear)/(zFar - zNear);
        frustumMatrix[2][3] = -2*zFar*zNear/(zFar-zNear);
        frustumMatrix[3][2] = -1;
        // here is important,as we need to push the matrix in stack, then later we can use it conveniently.
        MultMatrixf(frustumMatrix.columnMajor().coordinates);

        this->near = zNear;//store near value
        this->far = zFar; //store far value
        //we need use this two values for depth test
    } // Frustum()

// sets an orthographic projection matrix
void FakeGL::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    { // Ortho()
        //use the formula from our ppt
        if(left == right || bottom == top || zNear == zFar)
            return;
        Matrix4 orthogonalMatrix = Matrix4();
        orthogonalMatrix.SetZero();
        orthogonalMatrix[0][0] = 2/(right - left);
        orthogonalMatrix[0][3] = -1*(right + left)/(right - left);
        orthogonalMatrix[1][1] = 2/(top - bottom);
        orthogonalMatrix[1][3] = -1*(top + bottom)/(top - bottom);
        orthogonalMatrix[2][2] = -2/(zFar - zNear);
        orthogonalMatrix[2][3] = -1*(zFar + zNear)/(zFar-zNear);
        orthogonalMatrix[3][3] = 1;
        // here is important,as we need to push the matrix in stack, then later we can use it conveniently.
        MultMatrixf(orthogonalMatrix.columnMajor().coordinates);

        this->near = zNear; //store near value
        this->far = zFar; //store far value
        //we need use this two values for depth test

    } // Ortho()

// rotate the matrix
void FakeGL::Rotatef(float angle, float axisX, float axisY, float axisZ)
    { // Rotatef()
        //the function is different from our ppt, as we need to set a matrix that can do rotate for any axis
        //I find this formula on Internet.
        //but I found this function is not used in this program
        float theta = (angle / 180) * PI;
        float cosValue = cos(theta);
        float sinValue = sin(theta);
        Cartesian3 axisVector = Cartesian3(axisX,axisY,axisY);
        axisVector.unit();
        float x = axisVector.x;
        float y = axisVector.y;
        float z = axisVector.z;

        Matrix4 rotationMatrix = Matrix4();
        rotationMatrix.SetZero();

        rotationMatrix[0][0] = x * x * (1 - cosValue) + cosValue;
        rotationMatrix[0][1] = x * y * (1 - cosValue) + z * sinValue;
        rotationMatrix[0][2] = x * z * (1 - cosValue) - y * sinValue;
        rotationMatrix[1][0] = x * y * (1 - cosValue) - z * sinValue;
        rotationMatrix[1][1] = y * y * (1 - cosValue) + cosValue;
        rotationMatrix[1][2] = y * z * (1 - cosValue) + x * sinValue;
        rotationMatrix[2][0] = x * z * (1 - cosValue) + y * sinValue;
        rotationMatrix[2][1] = y * z * (1 - cosValue) - x * sinValue;
        rotationMatrix[2][2] = z * z * (1 - cosValue) + cosValue;
        rotationMatrix[3][3] = 1;

        // here is important,as we need to push the matrix in stack, then later we can use it conveniently.
        MultMatrixf(rotationMatrix.columnMajor().coordinates);

    } // Rotatef()

// scale the matrix
void FakeGL::Scalef(float xScale, float yScale, float zScale)
    { // Scalef()
        //use the formula from our ppt
        Matrix4 scaleMatrix = Matrix4();
        scaleMatrix.SetZero();
        scaleMatrix[0][0] = xScale;
        scaleMatrix[1][1] = yScale;
        scaleMatrix[2][2] = zScale;
        scaleMatrix[3][3] = 1;
        MultMatrixf(scaleMatrix.columnMajor().coordinates);
    } // Scalef()

// translate the matrix
void FakeGL::Translatef(float xTranslate, float yTranslate, float zTranslate)
    { // Translatef()
        //use the formula from our ppt
        Matrix4 translateMatrix = Matrix4();
        translateMatrix.SetIdentity();
        translateMatrix[0][3] = xTranslate;
        translateMatrix[1][3] = yTranslate;
        translateMatrix[2][3] = zTranslate;
        // here is important,as we need to push the matrix in stack, then later we can use it conveniently.
        MultMatrixf(translateMatrix.columnMajor().coordinates);

    } // Translatef()

// sets the viewport
void FakeGL::Viewport(int x, int y, int width, int height)
    { // Viewport()
        //set the viewport size and origin
        this->viewPortWidth = width;
        this->viewPortHeight = height;
        this->originScreenX = x;
        this->originScreenY = y;

    } // Viewport()

//-------------------------------------------------//
//                                                 //
// VERTEX ATTRIBUTE ROUTINES                       //
//                                                 //
//-------------------------------------------------//

// sets colour with floating point
void FakeGL::Color3f(float red, float green, float blue)
    { // Color3f()
    //guarantee red and green and blue are in [0,1]
        if (red < 0)
            red = 0;
        if (red > 1)
            red = 1;

        if (green < 0)
            green = 0;
        if (green > 1)
            green = 1;

        if (blue < 0)
            blue = 0;
        if (blue > 1)
            blue = 1;
        //then multiply them by 255, set RGB value.
        this->attributeColour.red = red * 255;
        this->attributeColour.green = green * 255;
        this->attributeColour.blue = blue * 255;

    } // Color3f()

// sets material properties
void FakeGL::Materialf(unsigned int parameterName, const float parameterValue)
    { // Materialf()
        //this function assign material and shininess exponent to corresponding var, according to different type.
        if (parameterName & FAKEGL_AMBIENT) {
            this->ambientMaterial[0] = this->ambientMaterial[1] = this->ambientMaterial[2] = parameterValue;
        }
        if (parameterName & FAKEGL_DIFFUSE) {
            this->diffuseMaterial[0] = this->diffuseMaterial[1] = this->diffuseMaterial[2] = parameterValue;
        }
        if (parameterName & FAKEGL_SPECULAR) {
            this->specularMaterial[0] = this->specularMaterial[1] = this->specularMaterial[2] = parameterValue;
        }
        if (parameterName & FAKEGL_EMISSION) {
            this->emissiveMaterial[0] = this->emissiveMaterial[1] = this->emissiveMaterial[2] = parameterValue;
        }
        if (parameterName & FAKEGL_SHININESS) {
            this->exponent = parameterValue;
        }


    } // Materialf()

void FakeGL::Materialfv(unsigned int parameterName, const float *parameterValues)
    { // Materialfv()
        //this function assign material to corresponding var, according to different type.
        if (parameterName & FAKEGL_AMBIENT) {
            this->ambientMaterial[0] = parameterValues[0];
            this->ambientMaterial[1] = parameterValues[1];
            this->ambientMaterial[2] = parameterValues[2];
            this->ambientMaterial[3] = parameterValues[3];
        }
        if (parameterName & FAKEGL_DIFFUSE) {
            this->diffuseMaterial[0] = parameterValues[0];
            this->diffuseMaterial[1] = parameterValues[1];
            this->diffuseMaterial[2] = parameterValues[2];
            this->diffuseMaterial[3] = parameterValues[3];
        }
        if (parameterName & FAKEGL_SPECULAR) {
            this->specularMaterial[0] = parameterValues[0];
            this->specularMaterial[1] = parameterValues[1];
            this->specularMaterial[2] = parameterValues[2];
            this->specularMaterial[3] = parameterValues[3];
        }
        if(parameterName & FAKEGL_EMISSION) {
            this->emissiveMaterial[0] = parameterValues[0];
            this->emissiveMaterial[1] = parameterValues[1];
            this->emissiveMaterial[2] = parameterValues[2];
            this->emissiveMaterial[3] = parameterValues[3];
        }
    } // Materialfv()

// sets the normal vector
void FakeGL::Normal3f(float x, float y, float z)
    { // Normal3f()
        //assign normal vector in. This function work together with Vertex3f.
        //And shoulder store a vertex's normal vector.
        this->attributeNormal.x = x;
        this->attributeNormal.y = y;
        this->attributeNormal.z = z;
    } // Normal3f()

// sets the texture coordinates
void FakeGL::TexCoord2f(float u, float v)
    { // TexCoord2f()
        //assign u v texture coordinates in.
        this->attributeU = u;
        this->attributeV = v;
    } // TexCoord2f()

// sets the vertex & launches it down the pipeline
void FakeGL::Vertex3f(float x, float y, float z)
    { // Vertex3f()
        //form a vertex's attributes
        vertexWithAttributes currentVertex = vertexWithAttributes();
        currentVertex.position = Homogeneous4(x, y, z, 1);
        currentVertex.colour = this->attributeColour;
        this->attributeNormal.unit();
        currentVertex.normal = Homogeneous4(this->attributeNormal);
        currentVertex.normal.w = 0;
        currentVertex.u = this->attributeU;
        currentVertex.v = this->attributeV;
        currentVertex.ambient = this->ambientMaterial;
        currentVertex.diffuse = this->diffuseMaterial;
        currentVertex.specular = this->specularMaterial;
        currentVertex.emissive = this->emissiveMaterial;
        currentVertex.exponent = this->exponent;
        //when forming a vertex attributes, we push it in this queue, as we then we can use this vertex later.
        this->vertexQueue.push_back(currentVertex);
        TransformVertex();//we need use TransformVertex function to transform the vertex in to screen coordinate system.
        if (RasterisePrimitive()){//according to the current primitive type, this step can rasterise a kind of primitive.
            while (this->fragmentQueue.empty() == false){
                //if the fragmentQueue is not empty we put all fragments in the framebuffer,which can let them shown on the screen.
                ProcessFragment();
            }
        }


    } // Vertex3f()

//-------------------------------------------------//
//                                                 //
// STATE VARIABLE ROUTINES                         //
//                                                 //
//-------------------------------------------------//

// disables a specific flag in the library
void FakeGL::Disable(unsigned int property)
    { // Disable()
        //this function is easy, just a switch
        if (property == FAKEGL_LIGHTING){
            this->isLighting = false;
        }
        else if (property == FAKEGL_TEXTURE_2D){
            this->isTexture = false;
        }
        else if (property == FAKEGL_DEPTH_TEST){
            this->isDepthTest = false;
        }
        else if (property == FAKEGL_PHONG_SHADING){
            this->isPhongShading = false;
        }
    } // Disable()

// enables a specific flag in the library
void FakeGL::Enable(unsigned int property)
    { // Enable()
        //this function is easy, just a switch
        if (property == FAKEGL_LIGHTING){
            this->isLighting = true;
        } else if (property == FAKEGL_TEXTURE_2D){
            this->isTexture = true;
        }
        else if (property == FAKEGL_DEPTH_TEST) {
            //here is important. we need set the size of depthBuffer, if we do not do this, we can not do depth test later.
            this->depthBuffer.Resize(this->frameBuffer.width, this->frameBuffer.height);
            this->isDepthTest = true;
        } else if (property == FAKEGL_PHONG_SHADING){
            this->isPhongShading = true;
        }
    } // Enable()

//-------------------------------------------------//
//                                                 //
// LIGHTING STATE ROUTINES                         //
//                                                 //
//-------------------------------------------------//

// sets properties for the one and only light
void FakeGL::Light(int parameterName, const float *parameterValues)
    { // Light()
        //according to different parameterName, we set corresponding values.
        if (parameterName & FAKEGL_POSITION){
            Homogeneous4 position = Homogeneous4(parameterValues[0], parameterValues[1], parameterValues[2], parameterValues[3]);
            //here we need transform the light position to VCS.
            this->lightPosition = this->stackModelView.back() * position;
            }

        if (parameterName & FAKEGL_AMBIENT){
            this->ambientLight[0] = parameterValues[0];
            this->ambientLight[1] = parameterValues[1];
            this->ambientLight[2] = parameterValues[2];
            this->ambientLight[3] = parameterValues[3];
        }
        if (parameterName & FAKEGL_DIFFUSE){
            this->diffuseLight[0] = parameterValues[0];
            this->diffuseLight[1] = parameterValues[1];
            this->diffuseLight[2] = parameterValues[2];
            this->diffuseLight[3] = parameterValues[3];
        }
        if (parameterName & FAKEGL_SPECULAR){
            this->specularLight[0] = parameterValues[0];
            this->specularLight[1] = parameterValues[1];
            this->specularLight[2] = parameterValues[2];
            this->specularLight[3] = parameterValues[3];
        }

    } // Light()

//-------------------------------------------------//
//                                                 //
// TEXTURE PROCESSING ROUTINES                     //
//                                                 //
// Note that we only allow one texture             //
// so glGenTexture & glBindTexture aren't needed   //
//                                                 //
//-------------------------------------------------//

// sets whether textures replace or modulate
void FakeGL::TexEnvMode(unsigned int textureMode)
    { // TexEnvMode()
        //just set the texture mode.
        if (textureMode & FAKEGL_REPLACE){
            this->textureState = FAKEGL_REPLACE;
        } else if (textureMode & FAKEGL_MODULATE){
            this->textureState = FAKEGL_MODULATE;
        }

    } // TexEnvMode()

// sets the texture image that corresponds to a given ID
void FakeGL::TexImage2D(const RGBAImage &textureImage)
    { // TexImage2D()
        //set the texture ppm data in texture Image. I got that on Internet, ppm and texture in GL have reversed coordinates.
        //So we change row and column.
        this->textureImage.Resize(textureImage.height, textureImage.width);
        for (int r = 0; r < textureImage.height; ++r) {
            for (int column = 0; column < textureImage.width; ++column) {
                this->textureImage[column][r] = textureImage[r][column];
            }
        }
    } // TexImage2D()

//-------------------------------------------------//
//                                                 //
// FRAME BUFFER ROUTINES                           //
//                                                 //
//-------------------------------------------------//

// clears the frame buffer
void FakeGL::Clear(unsigned int mask)
    { // Clear()
           //just set frame buffer and depth buffer to a same value.
           //here the mask can own both FAKEGL_COLOR_BUFFER_BIT and FAKEGL_DEPTH_BUFFER_BIT
           if(FAKEGL_COLOR_BUFFER_BIT & mask ){
                for (int r = 0; r < this->frameBuffer.height; r++) {
                    for (int column = 0; column < this->frameBuffer.width; column++) {
                        this->frameBuffer[r][column] = this->clearColour;
                    }
                }
            }

            if(FAKEGL_DEPTH_BUFFER_BIT & mask ){
                for (int r = 0; r < this->depthBuffer.height; r++) {
                    for (int column = 0; column < this->depthBuffer.width; column++) {
                        this->depthBuffer[r][column] = this->depthColour;
                    }
                }
            }

    } // Clear()

// sets the clear colour for the frame buffer
void FakeGL::ClearColor(float red, float green, float blue, float alpha)
    { // ClearColor()
        if (red < 0)
            red = 0;
        if (red > 1)
            red = 1;

        if (green < 0)
            green = 0;
        if (green > 1)
            green = 1;

        if (blue < 0)
            blue = 0;
        if (blue > 1)
            blue = 1;

        if (alpha < 0)
            alpha = 0;
        if (alpha > 1)
            alpha = 1;
        //guarantee the r g b and alpha are in [0-255]
        this->clearColour = RGBAValue(red*255, green*255, blue*255, alpha*255);
    } // ClearColor()

//-------------------------------------------------//
//                                                 //
// MAJOR PROCESSING ROUTINES                       //
//                                                 //
//-------------------------------------------------//

// transform one vertex & shift to the raster queue
void FakeGL::TransformVertex()
    { // TransformVertex()
        //every time we pop a vertex from the vertexQueue, then transform it.
        vertexWithAttributes currentVertex = this->vertexQueue.front();
        this->vertexQueue.pop_front();
        Homogeneous4 currentVCSCoordinates = this->stackModelView.back() * currentVertex.position; //to VCS
        Homogeneous4 currentCCSCoordinates = this->stackProjection.back() * currentVCSCoordinates; //to CCS
        Cartesian3 currentNDSCoordinates = currentCCSCoordinates.Point(); //to NDCS
        float scaleX = this->viewPortWidth * 0.5;
        float translateX = scaleX;
        float scaleY = this->viewPortHeight * 0.5;
        float translateY = scaleX;
        //to DCS, this step is different from our task in lab, as GL screen's origin is at the left bottom. So we do not need to inverse y.
        Cartesian3 currentDCSCoordinates = Cartesian3(currentNDSCoordinates.x * scaleX + translateX + this->originScreenX,
                                                      currentNDSCoordinates.y * scaleY + translateY + this->originScreenY,
                                                      -currentNDSCoordinates.z
                                                      );
        screenVertexWithAttributes currentScreenVertex = screenVertexWithAttributes();
        currentScreenVertex.position = currentDCSCoordinates;
        //here, it is important, we need store the VCS position for current vertex, as we need calculate the light under VCS later.
        if (currentVCSCoordinates.w !=0){
            currentScreenVertex.fragmentPosition = currentVCSCoordinates.Vector()/currentVCSCoordinates.w;
        } else{
            currentScreenVertex.fragmentPosition = currentVCSCoordinates.Vector();
        }

        currentScreenVertex.normal = (this->stackModelView.back() * currentVertex.normal).Vector();
        currentScreenVertex.colour = currentVertex.colour;
        currentScreenVertex.ambient = currentVertex.ambient;
        currentScreenVertex.diffuse = currentVertex.diffuse;
        currentScreenVertex.specular = currentVertex.specular;
        currentScreenVertex.emissive = currentVertex.emissive;
        currentScreenVertex.exponent = currentVertex.exponent;
        currentScreenVertex.u = currentVertex.u;
        currentScreenVertex.v = currentVertex.v;

        this->rasterQueue.push_back(currentScreenVertex);//push screen vertex in the raterQueue.


    } // TransformVertex()

// rasterise a single primitive if there are enough vertices on the queue
bool FakeGL::RasterisePrimitive()
    { // RasterisePrimitive()
        //according to different primitiveType, we rasterise them.
        //And the type is set outside.
        //raster one primitive , we need delete corresponding vertex.
        if (this->rasterQueue.empty()){
            return false;
        }
        if (this->primitiveType == FAKEGL_POINTS){
            RasterisePoint(*this->rasterQueue.begin());
            this->rasterQueue.pop_front();
            return true;
        }else if (this->primitiveType == FAKEGL_LINES) {
            if (this->rasterQueue.size() < 2)//we need a judgement to guarantee it's a line
                return false;
            RasteriseLineSegment(*this->rasterQueue.begin(), *(this->rasterQueue.begin() + 1));
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            return true;
        }else if (this->primitiveType == FAKEGL_TRIANGLES) {
            if (this->rasterQueue.size() < 3) {//we need a judgement to guarantee it's a triangle
                return false;
            }
            RasteriseTriangle(*this->rasterQueue.begin(), *(this->rasterQueue.begin() + 1), *(this->rasterQueue.begin() + 2));
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            return true;
        }
        return false;

    } // RasterisePrimitive()


// rasterises a single point
void FakeGL::RasterisePoint(screenVertexWithAttributes &vertex0)
    { // RasterisePoint()
        float halfPointSize = 0.5 * this->pointSize;
        //the position is the center of a point, so we can calculate the box of the point.
        float minX = vertex0.position.x - halfPointSize;
        float maxX = vertex0.position.x + halfPointSize;
        float minY = vertex0.position.y - halfPointSize;
        float maxY = vertex0.position.y + halfPointSize;

        fragmentWithAttributes currentFragment;
        //then calculate all fragments in the box.
        for (currentFragment.row = minY;  currentFragment.row < maxY; currentFragment.row++) {
            //start rwo from minY and end it at maxY
            if (currentFragment.row<0)
                continue;
            if (currentFragment.row >= this->frameBuffer.height)
                break;
            for (currentFragment.col = minX; currentFragment.col < maxX; currentFragment.col++) {
                //start column from minX and end it at maxX
                if (currentFragment.col<0)
                    continue;
                if (currentFragment.col>= this->frameBuffer.width)
                    break;

                Cartesian3 distanceVector = Cartesian3(
                        currentFragment.col - vertex0.position.x,
                        currentFragment.row - vertex0.position.y,
                        0
                        );
                //here we calculate the distance between current fragment and current vertex.
                //for saving resources, we just need to compare the distance square value rather than
                //the actual distance.
                //We compare the distance with radius square value.
                //if distanceSquare is greater than radiusSquare, then skip the curren fragment as it is
                //out of the point.
                float distanceSquare = distanceVector.dot(distanceVector);
                float radiusSquare = this->pointSize * this->pointSize;
                if (distanceSquare > radiusSquare)
                    continue;
                //if the fragment is in the point, then we set the vertex's colour and depth to it.
                //As all of the fragments form the point.
                currentFragment.colour = vertex0.colour;
                currentFragment.depth = vertex0.position.z;
                //then put the fragment in the fragmentQueue.
                //later we can put the fragment into the framebuffer,then it can be shown on the screen.
                this->fragmentQueue.push_back(currentFragment);
            }
        }

    } // RasterisePoint()

// rasterises a single line segment
void FakeGL::RasteriseLineSegment(screenVertexWithAttributes &vertex0, screenVertexWithAttributes &vertex1)
    { // RasteriseLineSegment()
        screenVertexWithAttributes currentInterpolateVertex = screenVertexWithAttributes();
        //store the point size, as we will change it for the line, but we should set it back after
        //completing the line rasterising.
        float tmpPointSize = this->pointSize;
        //set the point size as the half of the line width. As we need rasterise every point of the line.
        PointSize(this->lineWidth*0.5);
        float distance01Square = pow((vertex0.position.x - vertex1.position.x),2) +
                                 pow((vertex0.position.y - vertex1.position.y),2);
        float distance01 = sqrtf(distance01Square);
        float step = 1/distance01;
        //here it's just a line interpolation. a formula.
        for (float i = 0; i < 1.0; i += step) {
            currentInterpolateVertex.position = i * vertex0.position + (1-i) * vertex1.position;
            currentInterpolateVertex.colour = i * vertex0.colour + (1-i) * vertex1.colour;
            RasterisePoint(currentInterpolateVertex);
        }

        this->pointSize = tmpPointSize;// reset the point size.

    } // RasteriseLineSegment()

// rasterises a single triangle
void FakeGL::RasteriseTriangle(screenVertexWithAttributes &vertex0, screenVertexWithAttributes &vertex1, screenVertexWithAttributes &vertex2)
    { // RasteriseTriangle()
    // compute a bounding box that starts inverted to frame size
    // clipping will happen in the raster loop proper
    float minX = frameBuffer.width, maxX = 0.0;
    float minY = frameBuffer.height, maxY = 0.0;

    // test against all vertices
    if (vertex0.position.x < minX) minX = vertex0.position.x;
    if (vertex0.position.x > maxX) maxX = vertex0.position.x;
    if (vertex0.position.y < minY) minY = vertex0.position.y;
    if (vertex0.position.y > maxY) maxY = vertex0.position.y;

    if (vertex1.position.x < minX) minX = vertex1.position.x;
    if (vertex1.position.x > maxX) maxX = vertex1.position.x;
    if (vertex1.position.y < minY) minY = vertex1.position.y;
    if (vertex1.position.y > maxY) maxY = vertex1.position.y;

    if (vertex2.position.x < minX) minX = vertex2.position.x;
    if (vertex2.position.x > maxX) maxX = vertex2.position.x;
    if (vertex2.position.y < minY) minY = vertex2.position.y;
    if (vertex2.position.y > maxY) maxY = vertex2.position.y;

    // now for each side of the triangle, compute the line vectors
    Cartesian3 vector01 = vertex1.position - vertex0.position;
    Cartesian3 vector12 = vertex2.position - vertex1.position;
    Cartesian3 vector20 = vertex0.position - vertex2.position;

    // now compute the line normal vectors
    Cartesian3 normal01(-vector01.y, vector01.x, 0.0);
    Cartesian3 normal12(-vector12.y, vector12.x, 0.0);
    Cartesian3 normal20(-vector20.y, vector20.x, 0.0);

    // we don't need to normalise them, because the square roots will cancel out in the barycentric coordinates
    float lineConstant01 = normal01.dot(vertex0.position);
    float lineConstant12 = normal12.dot(vertex1.position);
    float lineConstant20 = normal20.dot(vertex2.position);

    // and compute the distance of each vertex from the opposing side
    float distance0 = normal12.dot(vertex0.position) - lineConstant12;
    float distance1 = normal20.dot(vertex1.position) - lineConstant20;
    float distance2 = normal01.dot(vertex2.position) - lineConstant01;

    // if any of these are zero, we will have a divide by zero error
    // but notice that if they are zero, the vertices are collinear in projection and the triangle is edge on
    // we can render that as a line, but the better solution is to render nothing.  In a surface, the adjacent
    // triangles will eventually take care of it
    if ((distance0 == 0) || (distance1 == 0) || (distance2 == 0))
        return;

    // create a fragment for reuse
    fragmentWithAttributes rasterFragment;
    //after calculating the light, colour, texture and material, we need adjust the colour for making
    //it closer to the  GL example. I searched this method on Internet, but I do not know why we need this
    //adjustment.
    //So here these two factor are set.
    float adjustExponent = 1.1;
    float adjustOffset = 45.0;
    //get the unit normal vector, later we need them for calculating light values.
    Cartesian3 normalVector0 = vertex0.normal.unit();
    Cartesian3 normalVector1 = vertex1.normal.unit();
    Cartesian3 normalVector2 = vertex2.normal.unit();

    Cartesian3 lightVector = Cartesian3();
    //if w of the light position is zero, then it's a directional light. we do not need to calculate
    //the light vector every time.
    if (this->lightPosition.w != 0){
        lightVector = this->lightPosition.Vector() / this->lightPosition.w;
    }
    else {
        lightVector = this->lightPosition.Vector();
    }
    //if we do not use phong shading, we need calculating every vertex's light intensity.
    float lightIntensity0[4], lightIntensity1[4], lightIntensity2[4] = {0,0,0,0};

    if (this->isLighting && !this->isPhongShading){
        //here we need to compare the lighting calculating is enabled.
        //but the phong shading is disabled.
        //Then we need to calculate ambient, diffuse, emissive and specular light values, and combine
        //them.
        if (this->lightPosition.w != 0){
            //w !=0 means the light is a point light, so we need calculate the light vector.
            lightVector = lightVector - vertex0.fragmentPosition;
        }
        lightVector = lightVector.unit();
        float diffuseCosValue = normalVector0.dot(lightVector);
        diffuseCosValue = max<float>(diffuseCosValue,0);
        //the eye is at origin, so use the position of VCS coordinate of the vertex,
        //we can get the vector starts at current vertex ends at eys. just like zero vector - fragmentPosition.
        Cartesian3 viewVector = -1*vertex0.fragmentPosition;
        Cartesian3 lightAndViewVector = lightVector + viewVector; // then add light vector and view vector.
        //then we can calculate the specular cosine value.
        float specularCosValue = normalVector0.dot(lightAndViewVector.unit());
        specularCosValue = max<float>(specularCosValue,0);//guarantee specularCosValue bigger than zero.
        for (int i = 0; i < 4; ++i) {
            //here, it's just a formula.
            lightIntensity0[i] = this->ambientLight[i] * vertex0.ambient[i] +
                    this->diffuseLight[i] * vertex0.diffuse[i] * diffuseCosValue+
                    this->specularLight[i] * vertex0.specular[i] * pow(specularCosValue, vertex0.exponent) +
                    vertex0.emissive[i];
        }
        //vertex1 is same as vertex0
        if (this->lightPosition.w != 0){
            lightVector = lightVector - vertex0.fragmentPosition;
        }
        lightVector = lightVector.unit();
        diffuseCosValue = normalVector1.dot(lightVector);
        diffuseCosValue = max<float>(diffuseCosValue,0);
        viewVector = -1*vertex1.fragmentPosition;
        lightAndViewVector = lightVector + viewVector;
        specularCosValue = normalVector1.dot(lightAndViewVector.unit());
        for (int i = 0; i < 4; ++i) {
            lightIntensity1[i] = this->ambientLight[i] * vertex1.ambient[i] +
                                 this->diffuseLight[i] * vertex1.diffuse[i] * diffuseCosValue+
                                 this->specularLight[i] * vertex1.specular[i] * pow(specularCosValue, vertex1.exponent) +
                                 vertex1.emissive[i];
        }
        //vertex2 is same as vertex0
        if (this->lightPosition.w != 0){
            lightVector = lightVector - vertex0.fragmentPosition;
        }
        lightVector = lightVector.unit();
        diffuseCosValue = normalVector2.dot(lightVector);
        diffuseCosValue = max<float>(diffuseCosValue,0);
        viewVector = -1*vertex2.fragmentPosition;
        lightAndViewVector = lightVector + viewVector;
        specularCosValue = normalVector2.dot(lightAndViewVector.unit());
        for (int i = 0; i < 4; ++i) {
            lightIntensity2[i] = this->ambientLight[i] * vertex2.ambient[i] +
                                 this->diffuseLight[i] * vertex2.diffuse[i] * diffuseCosValue+
                                 this->specularLight[i] * vertex2.specular[i] * pow(specularCosValue, vertex2.exponent) +
                                 vertex2.emissive[i];
        }

    }//if (this->isLighting && !this->isPhongShading)

    // loop through the pixels in the bounding box
    for (rasterFragment.row = minY; rasterFragment.row <= maxY; rasterFragment.row++)
        { // per row
        // this is here so that clipping works correctly
        if (rasterFragment.row < 0) continue;
        if (rasterFragment.row >= frameBuffer.height) continue;
        for (rasterFragment.col = minX; rasterFragment.col <= maxX; rasterFragment.col++)
            { // per pixel
            // this is also for correct clipping
            if (rasterFragment.col < 0) continue;
            if (rasterFragment.col >= frameBuffer.width) continue;

            // the pixel in cartesian format
            Cartesian3 pixel(rasterFragment.col, rasterFragment.row, 0.0);

            // right - we have a pixel inside the frame buffer AND the bounding box
            // note we *COULD* compute gamma = 1.0 - alpha - beta instead
            float alpha = (normal12.dot(pixel) - lineConstant12) / distance0;
            float beta = (normal20.dot(pixel) - lineConstant20) / distance1;
            float gamma = (normal01.dot(pixel) - lineConstant01) / distance2;

            // now perform the half-plane test
            if ((alpha < 0.0) || (beta < 0.0) || (gamma < 0.0))
                continue;
            //interpolate current fragment depth
            float currentFragmentDepth = alpha * vertex0.position.z + beta * vertex1.position.z + gamma * vertex2.position.z;
            //normalize the depth as we need multiply it by 255 and then compare the result with 255 to test the depth.
            rasterFragment.depth = (this->far - currentFragmentDepth) / (this->far - this->near);
            if (rasterFragment.depth > 1 || rasterFragment.depth < 0){
                continue;
            }
            if (this->isLighting){
                float currentRed, currentGreen, currentBlue, currentAlpha = 0;
                if (this->isPhongShading){
                    //phong shading is a formula.
                    //we need calculating light values for every fragment.
                    //by interpolating every normal and fragment position and exponent and ambient diffuse specular and emissive.
                    Cartesian3 currentFragmentNormal = alpha * normalVector0 + beta * normalVector1 + gamma * normalVector2;
                    Cartesian3 currentFragmentPosition = alpha * vertex0.fragmentPosition + beta * vertex1.position + gamma * vertex2.fragmentPosition;
                    float currentFragmentExponent = alpha * vertex0.exponent + beta * vertex1.exponent + gamma * vertex2.exponent;
                    float currentFragmentAmbient[4], currentFragmentDiffuse[4] ,currentFragmentSpecular[4] ,currentFragmentEmissive[4] = {0,0,0,0};
                    for (int i = 0; i < 4; ++i) {
                        currentFragmentAmbient[i] = alpha * vertex0.ambient[i] + beta * vertex1.ambient[i] + gamma * vertex2.ambient[i];
                        currentFragmentDiffuse[i] = alpha * vertex0.diffuse[i] + beta * vertex1.diffuse[i] + gamma * vertex2.diffuse[i];
                        currentFragmentSpecular[i] = alpha * vertex0.specular[i] + beta * vertex1.specular[i] + gamma * vertex2.specular[i];
                        currentFragmentEmissive[i] = alpha * vertex0.emissive[i] + beta * vertex1.emissive[i] + gamma * vertex2.emissive[i];
                    }
                    if (this->lightPosition.w != 0){
                        //w !=0 means the light is a point light, so we need calculate the light vector.
                        lightVector = lightVector - currentFragmentPosition;
                    }
                    lightVector = lightVector.unit();
                    float diffuseCosValue = currentFragmentNormal.dot(lightVector);
                    diffuseCosValue = max<float>(diffuseCosValue,0);
                    //the eye is at origin, so use the position of VCS coordinate of the vertex,
                    //we can get the vector starts at current vertex ends at eys. just like zero vector - fragmentPosition.
                    Cartesian3 viewVector = -1*currentFragmentPosition;
                    Cartesian3 lightAndViewVector = lightVector + viewVector; // then add light vector and view vector.
                    //then we can calculate the specular cosine value.
                    float specularCosValue = currentFragmentNormal.dot(lightAndViewVector.unit());
                    specularCosValue = max<float>(specularCosValue,0);//guarantee specularCosValue bigger than zero.
                    float currentFragmentLightIntensity[4] = {0,0,0,0};
                    for (int i = 0; i < 4; ++i) {
                        currentFragmentLightIntensity[i] = this->ambientLight[i] * currentFragmentAmbient[i] +
                                this->diffuseLight[i] * currentFragmentDiffuse[i] * diffuseCosValue +
                                this->specularLight[i] * currentFragmentSpecular[i] * pow(specularCosValue,currentFragmentExponent) +
                                currentFragmentEmissive[i];
                    }
                    //interpolate colour component
                    currentRed = alpha * vertex0.colour.red * currentFragmentLightIntensity[0] +
                            beta * vertex1.colour.red * currentFragmentLightIntensity[0] +
                            gamma * vertex2.colour.red * currentFragmentLightIntensity[0];
                    currentGreen = alpha * vertex0.colour.green * currentFragmentLightIntensity[1] +
                                 beta * vertex1.colour.green * currentFragmentLightIntensity[1] +
                                 gamma * vertex2.colour.green * currentFragmentLightIntensity[1];
                    currentBlue = alpha * vertex0.colour.blue * currentFragmentLightIntensity[2] +
                                   beta * vertex1.colour.blue * currentFragmentLightIntensity[2] +
                                   gamma * vertex2.colour.blue * currentFragmentLightIntensity[2];
                    currentAlpha = alpha * vertex0.colour.alpha * currentFragmentLightIntensity[3] +
                                   beta * vertex1.colour.alpha * currentFragmentLightIntensity[3] +
                                   gamma * vertex2.colour.alpha * currentFragmentLightIntensity[3];

                } //if (this->isPhongShading)
                else{
                    //if not phongshading, we do not need calculating every fragment's light values, just interpolating them by barycentric coordinates
                    currentRed = alpha * vertex0.colour.red * lightIntensity0[0] +
                                 beta * vertex1.colour.red * lightIntensity1[0] +
                                 gamma * vertex2.colour.red * lightIntensity2[0];
                    currentGreen = alpha * vertex0.colour.green * lightIntensity0[1] +
                                   beta * vertex1.colour.green * lightIntensity1[1] +
                                   gamma * vertex2.colour.green * lightIntensity2[1];
                    currentBlue = alpha * vertex0.colour.blue * lightIntensity0[2] +
                                  beta * vertex1.colour.blue * lightIntensity1[2] +
                                  gamma * vertex2.colour.blue * lightIntensity2[2];
                    currentAlpha = alpha * vertex0.colour.alpha * lightIntensity0[3] +
                                   beta * vertex1.colour.alpha * lightIntensity1[3] +
                                   gamma * vertex2.colour.alpha * lightIntensity2[3];

                }
                //adjust the colour then we can let the colour closer to the GL example.
                currentRed = pow(currentRed,adjustExponent) + adjustOffset;
                currentGreen = pow(currentGreen,adjustExponent) + adjustOffset;
                currentBlue = pow(currentBlue,adjustExponent) + adjustOffset;
                currentAlpha = pow(currentAlpha,adjustExponent) + adjustOffset;
                rasterFragment.colour.red = currentRed;
                rasterFragment.colour.green = currentGreen;
                rasterFragment.colour.blue = currentBlue;
                rasterFragment.colour.alpha = currentAlpha;
            } //if (this->isLighting)
            else{
                // compute colour
                rasterFragment.colour = alpha * vertex0.colour + beta * vertex1.colour + gamma * vertex2.colour;
            }
            //calculating the interpolation values of texture.if texture is enabled.
            if (this->isTexture){
                int uCoordinate = int ((alpha * vertex0.u + beta * vertex1.u + gamma * vertex2.u) * this->textureImage.height);
                int vCoordinate = int ((alpha * vertex0.v + beta * vertex1.v + gamma * vertex2.v) * this->textureImage.width);
                if (this->textureState & FAKEGL_MODULATE){
                    rasterFragment.colour = rasterFragment.colour.modulate(this->textureImage[uCoordinate][vCoordinate]);
                }else if (this->textureState & FAKEGL_REPLACE){
                    rasterFragment.colour = this->textureImage[uCoordinate][vCoordinate];
                }
            }
            // now we add it to the queue for fragment processing
            fragmentQueue.push_back(rasterFragment);
            } // per pixel
        } // per row
    } // RasteriseTriangle()

// process a single fragment
void FakeGL::ProcessFragment()
    { // ProcessFragment()
        fragmentWithAttributes currentFragment = this->fragmentQueue.front();
        this->fragmentQueue.pop_front();
        int currentRow = currentFragment.row;
        int currentCol = currentFragment.col;
        if (this->isDepthTest){
            //if depth test is enabled, we need compare the depth,and let smaller one go in to the framebuffer,
            // then it can be shown on the screen. And update the depthBuffer.
            float currentDepth = (float) this->depthBuffer[currentRow][currentCol].alpha;
            float currentFragmentDepth = currentFragment.depth * 255;
            if (currentFragmentDepth <= currentDepth){
                this->frameBuffer[currentRow][currentCol].red = currentFragment.colour.red;
                this->frameBuffer[currentRow][currentCol].green = currentFragment.colour.green;
                this->frameBuffer[currentRow][currentCol].blue = currentFragment.colour.blue;
                this->frameBuffer[currentRow][currentCol].alpha = currentFragment.colour.alpha;
                this->depthBuffer[currentRow][currentCol].alpha = currentFragmentDepth;
            }
        } else{
            this->frameBuffer[currentRow][currentCol].red = currentFragment.colour.red;
            this->frameBuffer[currentRow][currentCol].green = currentFragment.colour.green;
            this->frameBuffer[currentRow][currentCol].blue = currentFragment.colour.blue;
            this->frameBuffer[currentRow][currentCol].alpha = currentFragment.colour.alpha;
        }

    } // ProcessFragment()

// standard routine for dumping the entire FakeGL context (except for texture / image)
std::ostream &operator << (std::ostream &outStream, FakeGL &fakeGL)
    { // operator <<
    outStream << "=========================" << std::endl;
    outStream << "Dumping FakeGL Context   " << std::endl;
    outStream << "=========================" << std::endl;


    outStream << "-------------------------" << std::endl;
    outStream << "Vertex Queue:            " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto vertex = fakeGL.vertexQueue.begin(); vertex < fakeGL.vertexQueue.end(); vertex++)
        { // per matrix
        outStream << "Vertex " << vertex - fakeGL.vertexQueue.begin() << std::endl;
        outStream << *vertex;
        } // per matrix


    outStream << "-------------------------" << std::endl;
    outStream << "Raster Queue:            " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto vertex = fakeGL.rasterQueue.begin(); vertex < fakeGL.rasterQueue.end(); vertex++)
        { // per matrix
        outStream << "Vertex " << vertex - fakeGL.rasterQueue.begin() << std::endl;
        outStream << *vertex;
        } // per matrix


    outStream << "-------------------------" << std::endl;
    outStream << "Fragment Queue:          " << std::endl;
    outStream << "-------------------------" << std::endl;
    for (auto fragment = fakeGL.fragmentQueue.begin(); fragment < fakeGL.fragmentQueue.end(); fragment++)
        { // per matrix
        outStream << "Fragment " << fragment - fakeGL.fragmentQueue.begin() << std::endl;
        outStream << *fragment;
        } // per matrix


    return outStream;
    } // operator <<

// subroutines for other classes
std::ostream &operator << (std::ostream &outStream, vertexWithAttributes &vertex)
    { // operator <<
    std::cout << "Vertex With Attributes" << std::endl;
    std::cout << "Position:   " << vertex.position << std::endl;
    std::cout << "Colour:     " << vertex.colour << std::endl;

	// you

    return outStream;
    } // operator <<

std::ostream &operator << (std::ostream &outStream, screenVertexWithAttributes &vertex) 
    { // operator <<
    std::cout << "Screen Vertex With Attributes" << std::endl;
    std::cout << "Position:   " << vertex.position << std::endl;
    std::cout << "Colour:     " << vertex.colour << std::endl;

    return outStream;
    } // operator <<

std::ostream &operator << (std::ostream &outStream, fragmentWithAttributes &fragment)
    { // operator <<
    std::cout << "Fragment With Attributes" << std::endl;
    std::cout << "Row:        " << fragment.row << std::endl;
    std::cout << "Col:        " << fragment.col << std::endl;
    std::cout << "Colour:     " << fragment.colour << std::endl;

    return outStream;
    } // operator <<


    
    