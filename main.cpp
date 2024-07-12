#include "sources/civitai.hpp"
#include "sources/dataset.hpp"

int
main()
{
  // pxd::enhance("dataset_vars.json",
  // "dataset/female_positive_prompts.parquet");

  pxd::Dataset dataset;
  dataset.init("dataset/female_positive_prompts.parquet");

  dataset.read();

  return 0;
}