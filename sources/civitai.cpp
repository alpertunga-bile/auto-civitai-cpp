#include "civitai.hpp"

#include "dataset.hpp"

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include "format.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace pxd {

bool
set_civitai_variables(const std::string& dataset_vars_filepath,
                      CivitaiVariables& civitai_vars)
{
  if (!std::filesystem::exists(dataset_vars_filepath)) {
    fmt::println("{} is not exists", dataset_vars_filepath);
    return false;
  }

  std::ifstream json_file(dataset_vars_filepath);
  nlohmann::json data = nlohmann::json::parse(json_file);
  json_file.close();

  civitai_vars.hour_end = data["hour_end"];
  civitai_vars.minute_end = data["minute_end"];
  civitai_vars.image_limit = data["image_limit"];
  civitai_vars.sort = data["sort"];
  civitai_vars.period = data["period"];
  civitai_vars.nsfw = data["nsfw"];
  civitai_vars.start_cursor = data["start_cursor"];

  for (auto& wanted_prompt : data["wanted_prompts"]) {
    civitai_vars.wanted_prompts.emplace(wanted_prompt);
  }

  for (auto& unwanted_prompt : data["unwanted_prompts"]) {
    civitai_vars.unwanted_prompts.emplace(unwanted_prompt);
  }

  return true;
}

std::string
get_url(const CivitaiVariables& vars)
{
  std::string url =
    fmt::format("https://civitai.com/api/v1/images?limit={}&sort={}&period={}",
                vars.image_limit,
                vars.sort,
                vars.period);

  if (vars.nsfw != "All") {
    url = fmt::format("{}&nsfw={}", url, vars.nsfw);
  }

  if (vars.start_cursor != "") {
    url = fmt::format("{}&cursor={}", url, vars.start_cursor);
  }

  return url;
}

bool
get_json_object(const CivitaiVariables& vars, nlohmann::json& json_object)
{
  auto res = cpr::Get(cpr::Url(get_url(vars)));

  if (res.status_code != 200) {
    fmt::println("HTTP Get function is not worked");
    return false;
  }

  json_object = nlohmann::json::parse(res.text);

  return true;
}

bool
get_dataset_variables(const nlohmann::json& json_file,
                      int image_index,
                      std::string& prompt,
                      std::string& media_url)
{
  try {
    prompt = json_file["items"][image_index]["meta"]["prompt"];
  } catch (const std::exception& e) {
    fmt::println("Can't get the prompt");
    return false;
  }

  try {
    media_url = json_file["items"][image_index]["url"];
  } catch (const std::exception& e) {
    fmt::println("Can't get the media_url");
    return false;
  }

  return true;
}

void
get_current_hour_minute(int& current_hour, int& current_minute)
{
  const auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  auto local_tm = *localtime(&tt);

  current_hour = local_tm.tm_hour;
  current_minute = local_tm.tm_min;
}

void
print_vars(const CivitaiVariables& vars)
{
  fmt::println("{:#^100}", " PXD Civitai Variables ");

  fmt::println("{:30} : {}", "Hour End", vars.hour_end);
  fmt::println("{:30} : {}", "Minute End", vars.minute_end);
  fmt::println("{:30} : {}", "Image Limit", vars.image_limit);
  fmt::println("{:30} : {}", "Sort", vars.sort);
  fmt::println("{:30} : {}", "Period", vars.period);
  fmt::println("{:30} : {}", "NSFW", vars.nsfw);
  fmt::print("{:30} : ", "Wanted Prompts");

  for (auto& wanted_prompt : vars.wanted_prompts) {
    fmt::print("{}, ", wanted_prompt);
  }

  fmt::print("\n{:30} : ", "Unwanted Prompts");

  for (auto& unwanted_prompt : vars.unwanted_prompts) {
    fmt::print("{}, ", unwanted_prompt);
  }

  fmt::print("\n{:#^100}\n", "");
}

bool
enhance(const std::string& dataset_vars_filepath,
        const std::string& dataset_filepath)
{
  pxd::Dataset dataset;
  dataset.init(dataset_filepath.c_str());

  dataset.read();

  CivitaiVariables civitai_vars;

  if (!set_civitai_variables(dataset_vars_filepath, civitai_vars)) {
    fmt::println("Can't get the civitai variables");
    return false;
  }

  print_vars(civitai_vars);

  nlohmann::json json_object;

  if (!get_json_object(civitai_vars, json_object)) {
    fmt::println("Can't create the json object");
    return false;
  }

  int current_hour, current_minute;

  get_current_hour_minute(current_hour, current_minute);

  return true;
}

}