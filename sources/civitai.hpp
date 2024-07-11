#pragma once

#include <string>
#include <unordered_set>

namespace pxd {
struct CivitaiVariables
{
  int hour_end = 23;
  int minute_end = 30;
  int image_limit = 200;
  std::string sort = "Newest";
  std::string period = "AllTime";
  std::string nsfw = "All";
  std::string start_cursor = "";
  std::unordered_set<std::string> wanted_prompts;
  std::unordered_set<std::string> unwanted_prompts;
};

bool
enhance(const std::string& dataset_vars_filepath,
        const std::string& dataset_filepath);
}