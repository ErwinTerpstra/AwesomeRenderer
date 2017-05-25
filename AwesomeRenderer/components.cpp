#include "stdafx.h"
#include "transformation.h"
#include "model.h"
#include "renderable.h"
#include "arealight.h"

using namespace AwesomeRenderer;

int lastID = 0;

const int Transformation::id = lastID++;
const int Model::id = lastID++;
const int Renderable::id = lastID++;
const int AreaLight::id = lastID++;