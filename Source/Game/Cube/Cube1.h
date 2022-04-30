/*+===================================================================
  File:      CUBE1.H

  Summary:   Cube header file contains declarations of Cube class
             used for the lab samples of Game Graphics Programming
             course.

  Classes: Cube

  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    Cube1

  Summary:  A renderable 3d cube object

  Methods:  Update
              Overriden function that updates the cube every frame
            Cube
              Constructor.
            ~Cube
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class Cube1 : public BaseCube
{
public:
    Cube1(const std::filesystem::path& textureFilePath) = delete;   //QUESTION : 없애도 ㄱㅊ?
    Cube1(const Cube1& other) = delete;
    Cube1(Cube1&& other) = delete;
    Cube1& operator=(const Cube1& other) = delete;
    Cube1& operator=(Cube1&& other) = delete;
    ~Cube1() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};