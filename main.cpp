#include "sources/civitai.hpp"
#include "sources/dataset.hpp"

#include "argparse/argparse.hpp"

int
main(int argc, char* argv[])
{
  argparse::ArgumentParser program("auto-civitai-cpp");

  bool is_enhance = false;
  bool is_info = false;
  int row_count = 5;
  std::string dataset_path = "dataset/female_positive_prompts.parquet";
  std::string dataset_vars = "dataset_vars.json";

  if (!std::filesystem::exists("dataset")) {
    std::filesystem::create_directory("dataset");
  }

  program.add_argument("-e", "--enhance")
    .help("Enhance the dataset")
    .default_value(false)
    .implicit_value(true)
    .store_into(is_enhance);

  program.add_argument("-i", "--info")
    .help("Print information about the dataset")
    .default_value(false)
    .implicit_value(true)
    .store_into(is_info);

  program.add_argument("--row_count")
    .help("Print as row count from the head of the dataset")
    .scan<'i', int>()
    .default_value(row_count)
    .store_into(row_count);

  program.add_argument("-dp", "--dataset_path")
    .help("Dataset file path to read and write file")
    .default_value(dataset_path)
    .store_into(dataset_path);

  program.add_argument("-dv", "--dataset_vars")
    .help("Dataset variable json file path to get requried variables")
    .default_value(dataset_vars)
    .store_into(dataset_vars);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }

  if (is_enhance) {
    pxd::enhance(dataset_vars, dataset_path);
  } else if (is_info) {
    pxd::Dataset dataset;
    dataset.init(dataset_path.c_str());
    dataset.read();
    dataset.print(row_count);
  }

  return 0;
}