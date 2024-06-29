#include "dataset.hpp"

#include <filesystem>
namespace pxd {

arrow::Result<std::shared_ptr<arrow::Table>>
Dataset::create_default_schema()
{
  auto schema =
    arrow::schema({ arrow::field("prompt", arrow::utf8(), false),
                    arrow::field("media_url", arrow::utf8(), false) });

  return arrow::Table::MakeEmpty(schema);
}

void
Dataset::create(const char* dataset_name)
{
  m_dataset_name = dataset_name;

  if (std::filesystem::exists(dataset_name)) {
    read_all();
    return;
  }

  m_schema = arrow::schema({ arrow::field("prompt", arrow::utf8(), false),
                             arrow::field("media_url", arrow::utf8(), false) });

  m_table = arrow::Table::MakeEmpty(m_schema);
}

void
Dataset::read_all()
{
  auto file =
    arrow::io::ReadableFile::Open(m_dataset_name, arrow::default_memory_pool());

  std::unique_ptr<parquet::arrow::FileReader> reader;
  auto status =
    parquet::arrow::OpenFile(*file, arrow::default_memory_pool(), &reader);

  std::shared_ptr<arrow::Table> temp_table;
  status = reader->ReadTable(&temp_table);

  m_table = std::move(temp_table);
}

void
Dataset::write()
{
  auto file = arrow::io::FileOutputStream::Open(m_dataset_name);

  auto status = parquet::arrow::WriteTable(
    *m_table->get(), arrow::default_memory_pool(), *file);
}
}