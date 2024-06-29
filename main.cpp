#include "sources/dataset.hpp"

int
main()
{
  pxd::Dataset dataset;

  dataset.create();
  dataset.write("temp.parquet");

  return 0;
}