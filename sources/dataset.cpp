#include "dataset.hpp"

#include <filesystem>

namespace pxd {

arrow::Result<std::shared_ptr<arrow::Table>>
Dataset::create()
{
  m_status = m_prompts.Reserve(m_dataset_vals.size());
  m_status = m_media_urls.Reserve(m_dataset_vals.size());

  for (auto& [prompt, media_url] : m_dataset_vals) {
    m_status = m_prompts.Append(prompt);
    m_status = m_media_urls.Append(media_url);
  }

  std::shared_ptr<arrow::Array> prompt_arr;
  m_status = m_prompts.Finish(&prompt_arr);

  std::shared_ptr<arrow::Array> media_urls_arr;
  m_status = m_media_urls.Finish(&media_urls_arr);

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
  m_status =
    parquet::arrow::OpenFile(*file, arrow::default_memory_pool(), &reader);

  std::shared_ptr<arrow::Table> table;
  m_status = reader->ReadTable(&table);

  add_to<arrow::StringArray>(m_prompts, 0, table);
  add_to<arrow::StringArray>(m_media_urls, 1, table);

  for (int i = 0; i < m_prompts.length(); ++i) {
    if (m_dataset_vals.find(std::string(m_media_urls.GetView(i))) !=
        m_dataset_vals.end()) {
      continue;
    }

    m_dataset_vals[std::string(m_media_urls.GetView(i))] =
      std::string(m_prompts.GetView(i));
  }

  m_prompts.Reset();
  m_media_urls.Reset();
}

void
Dataset::write()
{
  auto file = arrow::io::FileOutputStream::Open(m_dataset_name);

  auto table = create();

  m_status = parquet::arrow::WriteTable(
    *table->get(), arrow::default_memory_pool(), *file);
}

void
Dataset::print(int row_count)
{
  int counter = 0;

  fmt::println("Total rows : {}", m_dataset_vals.size());

  fmt::println("{:^73} | {:^70}", "Prompt", "Media Url");

  for (auto& [prompt, media_url] : m_dataset_vals) {
    if (counter == row_count) {
      break;
    }

    fmt::println(
      "{:70}... | {:70}...", prompt.substr(0, 70), media_url.substr(0, 70));
    counter++;
  }
}

void
Dataset::add_row(std::string& prompt, std::string& media_url)
{
  if (m_dataset_vals.find(media_url) != m_dataset_vals.end()) {
    return;
  }

  m_dataset_vals[media_url] = prompt;
}
}