#include "sources/dataset.hpp"

#include <iostream>

int
main()
{
  pxd::Dataset dataset;

  dataset.create("temp.parquet");
  std::cout << dataset.get_table();

  return 0;
}