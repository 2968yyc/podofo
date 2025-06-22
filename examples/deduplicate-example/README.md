# PDF Object Deduplication Example

这个示例演示了如何使用PoDoFo库的`DeduplicateObjects`方法来去重PDF对象，类似于mutool的clean功能。

## 功能

`DeduplicateObjects`方法类似于mutool的`clean -gggg`功能，允许您：

- 检测重复的PDF对象
- 合并相同的对象
- 更新所有引用
- 清理未使用的对象
- 减少PDF文件大小

## 编译

```bash
mkdir build
cd build
cmake ..
make deduplicate-example create-test-pdf
```

## 使用方法

### 创建测试PDF

首先创建一个包含重复对象的测试PDF：

```bash
./create-test-pdf
```

这将创建一个名为`test-with-duplicates.pdf`的文件，包含多个重复对象。

### 执行去重

```bash
./deduplicate-example <input.pdf> <output.pdf> [--aggressive]
```

### 完整示例

```bash
# 1. 创建测试PDF
./create-test-pdf

# 2. 查看原始对象数量
./deduplicate-example test-with-duplicates.pdf temp.pdf

# 3. 执行标准去重
./deduplicate-example test-with-duplicates.pdf deduplicated-standard.pdf

# 4. 执行激进去重
./deduplicate-example test-with-duplicates.pdf deduplicated-aggressive.pdf --aggressive
```

## 去重过程

去重过程包括以下步骤：

1. **内容分析**：分析所有PDF对象的内容
2. **重复检测**：识别具有相同内容的对象
3. **引用更新**：更新所有引用以指向保留的对象
4. **对象删除**：删除重复的对象
5. **垃圾回收**：清理未引用的对象

## 去重模式

### 标准模式 (aggressive = false)
- 比较对象的基本结构
- 不包括流内容的比较
- 更快但可能遗漏一些重复

### 激进模式 (aggressive = true)
- 包括流内容的比较
- 更彻底的重复检测
- 可能发现更多重复对象
- 处理时间较长

## 注意事项

- 去重过程会修改PDF的内部结构
- 建议在去重前备份原始文件
- 某些PDF可能包含循环引用，去重过程会处理这些情况
- 去重后文件大小通常会减小

## API 参考

```cpp
void PdfMemDocument::DeduplicateObjects(bool aggressive = true);
```

### 参数

- `aggressive`: 是否执行激进去重
  - `true`: 包括流内容比较
  - `false`: 仅比较对象结构

### 行为

- 识别并合并重复的PDF对象
- 更新所有引用以指向保留的对象
- 删除重复对象
- 执行垃圾回收清理未引用的对象

## 与mutool的对比

| 功能 | mutool clean -gggg | PoDoFo实现 |
|------|-------------------|------------|
| 对象去重 | ✅ | ✅ |
| 引用更新 | ✅ | ✅ |
| 垃圾回收 | ✅ | ✅ |
| 流内容比较 | ✅ | ✅ (激进模式) |
| 循环引用处理 | ✅ | ✅ |

## 性能考虑

- 标准模式适合大多数情况
- 激进模式适合需要最大压缩的场景
- 大文件可能需要较长的处理时间
- 内存使用量与PDF对象数量成正比

## 测试

使用提供的测试程序可以验证去重功能：

1. `create-test-pdf`: 创建包含重复对象的测试PDF
2. `deduplicate-example`: 执行去重操作

测试PDF包含以下类型的重复对象：
- 重复的字符串对象
- 重复的数字对象
- 重复的数组对象
- 重复的字典对象
- 嵌套的重复对象 