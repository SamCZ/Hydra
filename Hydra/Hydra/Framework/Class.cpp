#include "Class.h"

int HClassDatabase::NextIndex = 0;
Map<String, List<String>> HClassDatabase::RawClassDB;

FastMap<String, int> HClassDatabase::ClassIndexMap;
FastMap<int, List<int>> HClassDatabase::ClassHiearchy;