#include "civitai.hpp"

#include "core.h"
#include "dataset.hpp"

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <unistd.h>

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

  return url;
}

bool
get_json_object(const CivitaiVariables& vars,
                const std::string& url,
                nlohmann::json& json_object)
{
  auto res = cpr::Get(cpr::Url(url));

  if (res.status_code != 200) {
    fmt::println("HTTP Get function is not worked");
    return false;
  }

  json_object = nlohmann::json::parse(res.text);

  return true;
}

bool
is_timeout(const int& hour_end, const int& minute_end)
{
  const auto now = std::chrono::system_clock::now();
  const auto tt = std::chrono::system_clock::to_time_t(now);
  const auto local_tm = *localtime(&tt);

  int current_hour = local_tm.tm_hour;
  int current_minute = local_tm.tm_min;

  return ((current_hour == hour_end && current_minute < minute_end) ||
          current_hour != hour_end);
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

std::string
string_replace(const std::string& str,
               const std::string& old_str,
               const std::string& new_str)
{
  return std::regex_replace(str, std::regex(old_str), new_str);
}

std::string
preprocess(const std::string& str)
{
  std::regex remove_lora_regex(",?\\s*<.+?>:?[0-9]*\\.?[0-9]*");
  std::regex remove_nonprompt_scalars_regex(",\\s*:[0-9]*\\.?[0-9]+");

  std::string temp_str = std::regex_replace(str, remove_lora_regex, "");
  temp_str = string_replace(temp_str, "\n", ", ");
  temp_str = string_replace(temp_str, "\t", " ");
  temp_str = std::regex_replace(temp_str, remove_nonprompt_scalars_regex, "");

  return temp_str;
}

bool
check_if_contains_word(const std::unordered_set<std::string>& word_list,
                       const std::string& str)
{
  std::regex delete_nonwords("[^a-zA-Z]+");
  std::string pure_string = std::regex_replace(str, delete_nonwords, " ");

  for (auto& word : word_list) {
    std::regex find_word_regex(fmt::format("\\b({})\\b", word));
    if (std::regex_search(
          pure_string.begin(), pure_string.end(), find_word_regex)) {
      return true;
    }
  }

  return false;
}

bool
check_prompt(const std::string& prompt,
             const std::unordered_set<std::string>& wanted_prompts,
             const std::unordered_set<std::string>& unwanted_prompts)
{
  std::string processed_prompt = preprocess(prompt);

  if (check_if_contains_word(unwanted_prompts, processed_prompt)) {
    return false;
  }

  if (check_if_contains_word(wanted_prompts, processed_prompt)) {
    return true;
  }

  return false;
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

  std::string current_cursor = civitai_vars.start_cursor;

  int check_error_counter = 0;

  while (is_timeout(civitai_vars.hour_end, civitai_vars.minute_end)) {

    std::string url;
    if (current_cursor == "") {
      url = get_url(civitai_vars);
    } else {
      url = fmt::format("{}&cursor={}", get_url(civitai_vars), current_cursor);
    }

    nlohmann::json json_object;

    while (check_error_counter != 3 &&
           !get_json_object(civitai_vars, url, json_object)) {
      fmt::println("Can't create the json object | Trying again ...");
      sleep(5);
      check_error_counter++;
    }

    if (check_error_counter == 3) {
      fmt::println("Loop is detected at {} cursor | Exiting ...",
                   current_cursor);
      return false;
    }

    fmt::println("Current cursor : {}", current_cursor);

    if (!json_object.contains("items")) {
      fmt::println("Json file is empty");
      return false;
    }

    int prompt_size = json_object["items"].size();

    for (int i = 0; i < prompt_size; ++i) {
      if (!json_object["items"][i].contains("meta")) {
        continue;
      }

      if (!json_object["items"][i]["meta"].contains("prompt")) {
        continue;
      }

      std::string prompt = json_object["items"][i]["meta"]["prompt"];
      std::string media_url = json_object["items"][i]["url"];

      if (!check_prompt(prompt,
                        civitai_vars.wanted_prompts,
                        civitai_vars.unwanted_prompts)) {
        continue;
      }

      std::string processed_prompt = preprocess(prompt);

      dataset.add_row(processed_prompt, media_url);
    }

    dataset.write();

    if (!json_object["metadata"].contains("nextCursor")) {
      fmt::println("Can't get the next cursor");
      return false;
    }

    if (json_object["metadata"]["nextCursor"].is_number_unsigned()) {
      current_cursor = std::to_string(
        json_object["metadata"]["nextCursor"].get<unsigned int>());
    } else if (json_object["metadata"]["nextCursor"].is_string()) {
      current_cursor = json_object["metadata"]["nextCursor"];
    }

    check_error_counter = 0;

    fmt::println("Waiting for 3 seconds ...");
    sleep(3);
  }

  return true;
}
}