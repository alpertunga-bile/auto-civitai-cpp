#pragma once

#include <arrow/api.h>
#include <arrow/io/api.h>

#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

namespace pxd {

class Dataset
{
public:
  static arrow::Result<std::shared_ptr<arrow::Table>> create_default_schema();

  void create(const char* dataset_name);

  void read_all();
  void write();

  inline arrow::Table* get_table() { return m_table->get(); }

private:
  arrow::Result<std::shared_ptr<arrow::Table>> m_table;
  std::shared_ptr<arrow::Schema> m_schema;

  std::string m_dataset_name;
};
}