#include "dataset.hpp"

#include <filesystem>

namespace pxd {

arrow::Result<std::shared_ptr<arrow::Table>>
Dataset::create()
{
  arrow::Status status;

  status = prompts.Reserve(dataset_vals.size());
  status = media_urls.Reserve(dataset_vals.size());

  for (auto& [prompt, media_url] : dataset_vals) {
    status = prompts.Append(prompt);
    status = media_urls.Append(media_url);
  }

  std::shared_ptr<arrow::Array> prompt_arr;
  status = prompts.Finish(&prompt_arr);

  std::shared_ptr<arrow::Array> media_urls_arr;
  status = media_urls.Finish(&media_urls_arr);

  auto schema =
    arrow::schema({ arrow::field("prompt", arrow::utf8(), false),
                    arrow::field("media_url", arrow::utf8(), false) });

  return arrow::Table::Make(schema, { prompt_arr, media_urls_arr });
}

void
Dataset::init(const char* dataset_name)
{
  m_dataset_name = dataset_name;
}

void
Dataset::read()
{
  if (!std::filesystem::exists(m_dataset_name)) {
    return;
  }

  auto file =
    arrow::io::ReadableFile::Open(m_dataset_name, arrow::default_memory_pool());

  std::unique_ptr<parquet::arrow::FileReader> reader;
  auto status =
    parquet::arrow::OpenFile(*file, arrow::default_memory_pool(), &reader);

  std::shared_ptr<arrow::Table> table;
  status = reader->ReadTable(&table);

  add_to<arrow::StringArray>(prompts, 0, table);
  add_to<arrow::StringArray>(media_urls, 1, table);

  for (int i = 0; i < prompts.length(); ++i) {
    if (dataset_vals.find(media_urls.GetView(i)) != dataset_vals.end()) {
      continue;
    }

    dataset_vals[media_urls.GetView(i)] = prompts.GetView(i);
  }

  prompts.Reset();
  media_urls.Reset();
}

void
Dataset::write()
{
  auto file = arrow::io::FileOutputStream::Open(m_dataset_name);

  auto table = create();

  auto status = parquet::arrow::WriteTable(
    *table->get(), arrow::default_memory_pool(), *file);
}

void
Dataset::add_row(std::string_view prompt, std::string_view media_url)
{
  if (dataset_vals.find(media_url) != dataset_vals.end()) {
    return;
  }

  dataset_vals[media_url] = prompt;
}
}