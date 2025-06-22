# PDF Page Selection Example

这个示例演示了如何使用PoDoFo库的`Select`方法来重新排列PDF页面。

## 功能

`Select`方法类似于PyMuPDF库的`Document.select()`函数，允许您：

- 选择特定的页面
- 重新排列页面顺序
- 复制页面（通过重复页面编号）
- 过滤掉无效的页面编号

## 编译

```bash
mkdir build
cd build
cmake ..
make select-example
```

## 使用方法

```bash
./select-example <input.pdf> <output.pdf> [page_numbers...]
```

### 示例

1. **重新排列页面**：
   ```bash
   ./select-example input.pdf output.pdf 0 2 1 3
   ```
   这将把页面重新排列为：第0页、第2页、第1页、第3页

2. **选择特定页面**：
   ```bash
   ./select-example input.pdf output.pdf 0 3 5
   ```
   这将只保留第0、3、5页

3. **复制页面**：
   ```bash
   ./select-example input.pdf output.pdf 0 0 1 1 2
   ```
   这将创建：第0页的副本、第1页的副本、第2页

4. **保持原顺序**：
   ```bash
   ./select-example input.pdf output.pdf
   ```
   不提供页面编号将保持所有页面的原始顺序

## 注意事项

- 页面编号从0开始
- 无效的页面编号会被忽略
- 重复的页面编号是允许的
- 如果所有页面编号都无效，文档将被清空

## API 参考

```cpp
void PdfMemDocument::Select(const std::vector<unsigned>& pageNumbers);
```

### 参数

- `pageNumbers`: 页面编号的向量（0基索引）
  - 如果为空，保持所有页面的当前顺序
  - 如果页面编号超出范围，将被忽略
  - 允许重复的页面编号

### 行为

- 根据提供的页面编号重新排序文档中的页面
- 未包含在pageNumbers向量中的页面将被删除
- 结果文档中的页面顺序将与pageNumbers中的顺序匹配 