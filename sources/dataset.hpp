#pragma once

#include <arrow/api.h>
#include <arrow/io/api.h>

#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>

#include <unordered_map>

namespace pxd {

class Dataset
{
public:
  void init(const char* dataset_name);

  void read();
  void write();
  void print(int row_count = 5);

  void add_row(std::string& prompt, std::string& media_url);

private:
  arrow::Result<std::shared_ptr<arrow::Table>> create();

  template<typename T, typename B>
  void add_to(B& builder,
              int column_index,
              std::shared_ptr<arrow::Table> table);

private:
  arrow::StringBuilder prompts;
  arrow::StringBuilder media_urls;

  std::unordered_map<std::string, std::string> dataset_vals;

  std::string m_dataset_name;
};

template<typename T, typename B>
inline void
Dataset::add_to(B& builder,
                int column_index,
                std::shared_ptr<arrow::Table> table)
{
  auto column = table->column(column_index);
  int num_chunks = column->num_chunks();

  for (int chunk_index = 0; chunk_index < num_chunks; ++chunk_index) {
    auto array = std::static_pointer_cast<T>(column->chunk(chunk_index));
    int64_t length = array->length();

    for (int64_t i = 0; i < length; ++i) {
      auto status = builder.Append(array->Value(i));
    }
  }
}
}