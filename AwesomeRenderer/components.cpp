#include "transformation.h"
#include "model.h"
#include "renderable.h"

using namespace AwesomeRenderer;

int lastID = 0;

const int Transformation::id = lastID++;
const int Model::id = lastID++;
const int Renderable::id = lastID++;