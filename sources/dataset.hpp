#pragma once

#include <arrow/api.h>
#include <arrow/dataset/api.h>

#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

namespace pxd {

class Dataset
{
public:
  static arrow::Result<std::shared_ptr<arrow::Table>> create_default_schema();

  bool create();

  void write(const char* dataset_name);

private:
  arrow::Result<std::shared_ptr<arrow::Table>> m_table;
  std::shared_ptr<arrow::Schema> m_schema;
  arrow::Result<std::shared_ptr<arrow::fs::FileSystem>> m_fs;
};
}