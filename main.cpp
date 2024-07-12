#include "sources/civitai.hpp"
#include "sources/dataset.hpp"

int
main()
{
  pxd::enhance("dataset_vars.json", "dataset/female_positive_prompts.parquet");

  return 0;
}