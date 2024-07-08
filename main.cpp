#include "sources/dataset.hpp"

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include <iostream>

int
main()
{
  /*
  pxd::Dataset dataset;
  dataset.init("temp.parquet");

  dataset.read();

  dataset.add_row("kasjd", "ajhsjdhqw");

  dataset.write();
  */

  auto res = cpr::Get(cpr::Url(
    "https://civitai.com/api/v1/images?limit=200&sort=Newest&period=AllTime"));

  nlohmann::json data = nlohmann::json::parse(res.text);

  std::cout << data["items"][0]["meta"]["prompt"];

  return 0;
}