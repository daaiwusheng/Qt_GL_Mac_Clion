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
//-------------------------------------------------//
//                                                 //
// CONSTRUCTOR / DESTRUCTOR                        //
//                                                 //
//-------------------------------------------------//

// constructor
FakeGL::FakeGL()
    { // constructor

        this->viewPortWidth = 0;
        this->viewPortHeight = 0;

        this->attribureColour.red = this->attribureColour.green = this->attribureColour.blue = this->attribureColour.alpha = 1.0;
        this->depthColour.red =  this->depthColour.green =  this->depthColour.blue = 0;
        this->depthColour.alpha = 255;

        this->attribureNormal.x = 0;
        this->attribureNormal.y = 0;
        this->attribureNormal.z = 1;

        Matrix4 identifyMatrix = Matrix4();
        identifyMatrix.SetIdentity();
        this->stackModelView.push_back(identifyMatrix);
        this->stackProjection.push_back(identifyMatrix);

        this->isLighting = false;
        this->isTexture = false;
        this->isDepthTest = false;
        this->isPhongShading = true;

        this->lightPosition = Homogeneous4(0,0,1,0);
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
        this->attribureV = 0;

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
        this->primitiveType = PrimitiveType;
    } // Begin()

// ends a sequence of geometric primitives
void FakeGL::End()
    { // End()
        this->primitiveType = -1;
    } // End()

// sets the size of a point for drawing
void FakeGL::PointSize(float size)
    { // PointSize()
        this->pointSize = round(size);
        if (this->pointSize < 1)
            this->pointSize = 1;
    } // PointSize()

// sets the width of a line for drawing purposes
void FakeGL::LineWidth(float width)
    { // LineWidth()
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
        this->matrixState = whichMatrix;
    } // MatrixMode()

// pushes a matrix on the stack
void FakeGL::PushMatrix()
    { // PushMatrix()
        if (this->matrixState & FAKEGL_MODELVIEW){
            this->stackModelView.push_back(this->stackModelView.back());
        }

        if (this->matrixState & FAKEGL_PROJECTION){
            this->stackProjection.push_back(this->stackProjection.back());
        }
    } // PushMatrix()

// pops a matrix off the stack
void FakeGL::PopMatrix()
    { // PopMatrix()
        if (this->matrixState & FAKEGL_MODELVIEW) {
            this->stackModelView.pop_back();
        }

        if (this->matrixState & FAKEGL_PROJECTION){
            this->stackProjection.pop_back();
        }

    } // PopMatrix()

// load the identity matrix
void FakeGL::LoadIdentity()
    { // LoadIdentity()
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
        MultMatrixf(frustumMatrix.columnMajor().coordinates);

        this->near = zNear;
        this->far = zFar;
    } // Frustum()

// sets an orthographic projection matrix
void FakeGL::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    { // Ortho()
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
        MultMatrixf(orthogonalMatrix.columnMajor().coordinates);

        this->near = zNear;
        this->far = zFar;

    } // Ortho()

// rotate the matrix
void FakeGL::Rotatef(float angle, float axisX, float axisY, float axisZ)
    { // Rotatef()
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


        MultMatrixf(rotationMatrix.columnMajor().coordinates);

    } // Rotatef()

// scale the matrix
void FakeGL::Scalef(float xScale, float yScale, float zScale)
    { // Scalef()
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
        Matrix4 translateMatrix = Matrix4();
        translateMatrix.SetIdentity();
        translateMatrix[0][3] = xTranslate;
        translateMatrix[1][3] = yTranslate;
        translateMatrix[2][3] = zTranslate;

        MultMatrixf(translateMatrix.columnMajor().coordinates);

    } // Translatef()

// sets the viewport
void FakeGL::Viewport(int x, int y, int width, int height)
    { // Viewport()

        float size = (width < height) ? width : height;
        this->viewPortWidth = size;
        this->viewPortHeight = size;
        this->originScreenX = x + (width / 2.0 - size / 2.0);
        this->originScreenY = y + (height / 2.0 - size / 2.0);

    } // Viewport()

//-------------------------------------------------//
//                                                 //
// VERTEX ATTRIBUTE ROUTINES                       //
//                                                 //
//-------------------------------------------------//

// sets colour with floating point
void FakeGL::Color3f(float red, float green, float blue)
    { // Color3f()
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

        this->attribureColour.red = red * 255;
        this->attribureColour.green = green * 255;
        this->attribureColour.blue = blue * 255;

    } // Color3f()

// sets material properties
void FakeGL::Materialf(unsigned int parameterName, const float parameterValue)
    { // Materialf()
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
        this->attribureNormal.x = x;
        this->attribureNormal.y = y;
        this->attribureNormal.z = z;
    } // Normal3f()

// sets the texture coordinates
void FakeGL::TexCoord2f(float u, float v)
    { // TexCoord2f()
        this->attributeU = u;
        this->attribureV = v;
    } // TexCoord2f()

// sets the vertex & launches it down the pipeline
void FakeGL::Vertex3f(float x, float y, float z)
    { // Vertex3f()
        vertexWithAttributes currentVertex = vertexWithAttributes();
        currentVertex.position = Homogeneous4(x, y, z, 1);
        currentVertex.colour = this->attribureColour;
        this->attribureNormal.unit();
        currentVertex.normal = Homogeneous4(this->attribureNormal);
        currentVertex.normal.w = 0;
        currentVertex.u = this->attributeU;
        currentVertex.v = this->attribureV;
        currentVertex.ambient = this->ambientMaterial;
        currentVertex.diffuse = this->diffuseMaterial;
        currentVertex.specular = this->specularMaterial;
        currentVertex.emissive = this->emissiveMaterial;
        currentVertex.exponent = this->exponent;

        this->vertexQueue.push_back(currentVertex);
        TransformVertex();
        if (RasterisePrimitive()){
            while (this->fragmentQueue.empty() == false){
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
        if (property & FAKEGL_LIGHTING){
            this->isLighting = false;
        }
        if (property & FAKEGL_TEXTURE_2D){
            this->isTexture = false;
        }
        if (property & FAKEGL_DEPTH_TEST){
            this->isDepthTest = false;
        }
        if (property & FAKEGL_PHONG_SHADING){
            this->isPhongShading = false;
        }
    } // Disable()

// enables a specific flag in the library
void FakeGL::Enable(unsigned int property)
    { // Enable()
        if (property & FAKEGL_LIGHTING){
            this->isLighting = true;
        }
        if (property & FAKEGL_TEXTURE_2D){
            this->isTexture = true;
        }
        if (property & FAKEGL_DEPTH_TEST) {
            this->depthBuffer.Resize(this->frameBuffer.width, this->frameBuffer.height);
            this->isDepthTest = true;
        }
        if (property & FAKEGL_PHONG_SHADING){
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
        if (parameterName & FAKEGL_POSITION){
            Homogeneous4 position = Homogeneous4(parameterValues[0], parameterValues[1], parameterValues[2], parameterValues[3]);
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
        if (textureMode & FAKEGL_REPLACE){
            this->textureState = FAKEGL_REPLACE;
        }
        if (textureMode & FAKEGL_MODULATE){
            this->textureState = FAKEGL_MODULATE;
        }

    } // TexEnvMode()

// sets the texture image that corresponds to a given ID
void FakeGL::TexImage2D(const RGBAImage &textureImage)
    { // TexImage2D()
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

           if(FAKEGL_COLOR_BUFFER_BIT & mask ){
                for (int r = 0; r < this->frameBuffer.height; ++r) {
                    for (int column = 0; column < this->frameBuffer.width; ++column) {
                        this->frameBuffer[r][column] = this->clearColour;
                    }
                }
            }

            if(FAKEGL_DEPTH_BUFFER_BIT & mask ){
                for (int r = 0; r < this->depthBuffer.height; ++r) {
                    for (int column = 0; column < this->depthBuffer.width; ++column) {
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
        vertexWithAttributes currentVertex = this->vertexQueue.front();
        this->vertexQueue.pop_front();
        Homogeneous4 currentVCSCoordinates = this->stackModelView.back() * currentVertex.position;
        Homogeneous4 currentCCSCoordinates = this->stackProjection.back() * currentVCSCoordinates;
        Cartesian3 currentNDSCoordinates = currentCCSCoordinates.Point();
        float scaleX = this->viewPortWidth * 0.5;
        float translateX = scaleX;
        float scaleY = this->viewPortHeight * 0.5;
        float translateY = scaleX;
        Cartesian3 currentDCSCoordinates = Cartesian3(currentNDSCoordinates.x * scaleX + translateX + this->originScreenX,
                                                      currentNDSCoordinates.y * scaleY + translateY + this->originScreenY,
                                                      currentNDSCoordinates.z
                                                      );
        screenVertexWithAttributes currentScreenVertex = screenVertexWithAttributes();
        currentScreenVertex.position = currentDCSCoordinates;
        currentScreenVertex.normal = (this->stackModelView.back() * currentVertex.normal).Vector();
        currentScreenVertex.colour = currentVertex.colour;
        currentScreenVertex.ambient = currentVertex.ambient;
        currentScreenVertex.diffuse = currentVertex.diffuse;
        currentScreenVertex.specular = currentVertex.specular;
        currentScreenVertex.emissive = currentVertex.emissive;
        currentScreenVertex.exponent = currentVertex.exponent;

        this->rasterQueue.push_back(currentScreenVertex);


    } // TransformVertex()

// rasterise a single primitive if there are enough vertices on the queue
bool FakeGL::RasterisePrimitive()
    { // RasterisePrimitive()
        if (this->rasterQueue.empty()){
            return false;
        }
        if (this->primitiveType & FAKEGL_POINTS){
            RasterisePoint(*this->rasterQueue.begin());
            this->rasterQueue.pop_front();
            return true;
        }
        if (this->primitiveType & FAKEGL_LINES) {
            if (this->rasterQueue.size() < 2)
                return false;
            RasteriseLineSegment(*this->rasterQueue.begin(), *(this->rasterQueue.begin() + 1));
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            return true;
        }
        if (this->primitiveType & FAKEGL_TRIANGLES) {
            if (this->rasterQueue.size() < 3) {
                return false;
            }
            RasteriseTriangle(*this->rasterQueue.begin(), *(this->rasterQueue.begin() + 1), *(this->rasterQueue.begin() + 2));
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            this->rasterQueue.pop_front();
            return true;
        }

    } // RasterisePrimitive()

// rasterises a single point
void FakeGL::RasterisePoint(screenVertexWithAttributes &vertex0)
    { // RasterisePoint()
        float halfPointSize = 0.5 * this->pointSize;
        float minX = vertex0.position.x - halfPointSize;
        float maxX = vertex0.position.x + halfPointSize;
        float minY = vertex0.position.y - halfPointSize;
        float maxY = vertex0.position.y + halfPointSize;

        fragmentWithAttributes currentFragment;
        for (currentFragment.row = minY;  currentFragment.row < maxY; currentFragment.row++) {
            if (currentFragment.row<0)
                continue;
            if (currentFragment.row >= this->frameBuffer.height)
                break;
            for (currentFragment.col = minX; currentFragment.col < maxX; currentFragment.col++) {
                if (currentFragment.col<0)
                    continue;
                if (currentFragment.col>= this->frameBuffer.width)
                    break;

                Cartesian3 distanceVector = Cartesian3(
                        currentFragment.col - vertex0.position.x,
                        currentFragment.row - vertex0.position.y,
                        0
                        );
                float distanceSquare = distanceVector.dot(distanceVector);
                float radiusSquare = this->pointSize * this->pointSize;
                if (distanceSquare > radiusSquare)
                    continue;
                currentFragment.colour = vertex0.colour;
                currentFragment.depth = vertex0.position.z;
                this->fragmentQueue.push_back(currentFragment);
            }
        }

    } // RasterisePoint()

// rasterises a single line segment
void FakeGL::RasteriseLineSegment(screenVertexWithAttributes &vertex0, screenVertexWithAttributes &vertex1)
    { // RasteriseLineSegment()
        screenVertexWithAttributes currentInterpolateVertex = screenVertexWithAttributes();
        float tmpPointSize = this->pointSize;
        PointSize(this->lineWidth*0.5);
        float distance01Square = pow((vertex0.position.x - vertex1.position.x),2) +
                                 pow((vertex0.position.y - vertex1.position.y),2);
        float distance01 = sqrtf(distance01Square);
        float step = 1/distance01;

        for (float i = 0; i < 1.0; i += step) {
            currentInterpolateVertex.position = i * vertex0.position + (1-i) * vertex1.position;
            currentInterpolateVertex.colour = i * vertex0.colour + (1-i) * vertex1.colour;
            RasterisePoint(currentInterpolateVertex);
        }

        this->pointSize = tmpPointSize;

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

            // compute colour
            rasterFragment.colour = alpha * vertex0.colour + beta * vertex1.colour + gamma * vertex2.colour;

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


    
    