#include "dataset.hpp"

namespace pxd {

void
write_dataset(const arrow::Table& table, const char* dataset_name)
{
  char init_path[256];
  char* result = getcwd(init_path, 256);
  if (result == nullptr) {
    return;
  }

  auto fs = arrow::fs::FileSystemFromUriOrPath(init_path);
}

arrow::Result<std::shared_ptr<arrow::Table>>
Dataset::create_default_schema()
{
  auto schema =
    arrow::schema({ arrow::field("prompt", arrow::utf8(), false),
                    arrow::field("media_url", arrow::utf8(), false) });

  return arrow::Table::MakeEmpty(schema);
}

bool
Dataset::create()
{
  m_schema = arrow::schema({ arrow::field("prompt", arrow::utf8(), false),
                             arrow::field("media_url", arrow::utf8(), false) });

  m_table = arrow::Table::MakeEmpty(m_schema);

  char init_path[256];
  char* result = getcwd(init_path, 256);
  if (result == nullptr) {
    return false;
  }

  m_fs = arrow::fs::FileSystemFromUriOrPath(init_path);

  return true;
}

void
Dataset::write(const char* dataset_name)
{
  auto output = m_fs->get()->OpenOutputStream(dataset_name);
  auto status = parquet::arrow::WriteTable(
    *(m_table->get()), arrow::default_memory_pool(), *output);
}
}