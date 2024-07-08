#include "sources/dataset.hpp"

#include <iostream>

int
main()
{
  pxd::Dataset dataset;
  dataset.init("temp.parquet");

  dataset.read();

  dataset.add_row("kasjd", "ajhsjdhqw");

  dataset.write();

  return 0;
}