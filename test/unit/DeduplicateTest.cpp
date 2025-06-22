/**
 * SPDX-FileCopyrightText: (C) 2024 AI Assistant
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <PdfTest.h>

using namespace std;
using namespace PoDoFo;

TEST_CASE("DeduplicateEmptyDocument")
{
    PdfMemDocument doc;
    
    // Test with empty document - should not crash
    doc.DeduplicateObjects();
    REQUIRE(doc.GetObjects().GetSize() == 0);
    
    doc.DeduplicateObjects(true);  // aggressive mode
    REQUIRE(doc.GetObjects().GetSize() == 0);
}

TEST_CASE("DeduplicateSimpleObjects")
{
    PdfMemDocument doc;
    
    // Create some duplicate objects
    auto& obj1 = doc.GetObjects().CreateObject(PdfObject(42));
    auto& obj2 = doc.GetObjects().CreateObject(PdfObject(42));
    auto& obj3 = doc.GetObjects().CreateObject(PdfObject(100));
    
    unsigned initialSize = doc.GetObjects().GetSize();
    REQUIRE(initialSize >= 3);
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // The duplicate integer objects should have been merged
    bool found42 = false;
    bool found100 = false;
    
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsNumber() && obj->GetInt64() == 42)
            found42 = true;
        else if (obj->IsNumber() && obj->GetInt64() == 100)
            found100 = true;
    }
    
    REQUIRE(found42);
    REQUIRE(found100);
}

TEST_CASE("DeduplicateStringObjects")
{
    PdfMemDocument doc;
    
    // Create duplicate string objects
    auto& obj1 = doc.GetObjects().CreateObject(PdfObject(PdfString("Hello")));
    auto& obj2 = doc.GetObjects().CreateObject(PdfObject(PdfString("Hello")));
    auto& obj3 = doc.GetObjects().CreateObject(PdfObject(PdfString("World")));
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // Count unique strings
    int helloCount = 0;
    int worldCount = 0;
    
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsString())
        {
            if (obj->GetString().GetString() == "Hello")
                helloCount++;
            else if (obj->GetString().GetString() == "World")
                worldCount++;
        }
    }
    
    REQUIRE(helloCount == 1);  // Should only have one "Hello" object
    REQUIRE(worldCount == 1);  // Should only have one "World" object
}

TEST_CASE("DeduplicateArrayObjects")
{
    PdfMemDocument doc;
    
    // Create duplicate array objects
    PdfArray arr1;
    arr1.Add(PdfObject(1));
    arr1.Add(PdfObject(2));
    arr1.Add(PdfObject(3));
    
    PdfArray arr2;
    arr2.Add(PdfObject(1));
    arr2.Add(PdfObject(2));
    arr2.Add(PdfObject(3));
    
    auto& obj1 = doc.GetObjects().CreateObject(arr1);
    auto& obj2 = doc.GetObjects().CreateObject(arr2);
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // Should only have one array object
    int arrayCount = 0;
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsArray())
            arrayCount++;
    }
    
    REQUIRE(arrayCount == 1);
}

TEST_CASE("DeduplicateDictionaryObjects")
{
    PdfMemDocument doc;
    
    // Create duplicate dictionary objects
    PdfDictionary dict1;
    dict1.AddKey("Key1", PdfObject(100));
    dict1.AddKey("Key2", PdfObject(PdfString("Value")));
    
    PdfDictionary dict2;
    dict2.AddKey("Key1", PdfObject(100));
    dict2.AddKey("Key2", PdfObject(PdfString("Value")));
    
    auto& obj1 = doc.GetObjects().CreateObject(dict1);
    auto& obj2 = doc.GetObjects().CreateObject(dict2);
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // Should only have one dictionary object
    int dictCount = 0;
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsDictionary())
            dictCount++;
    }
    
    REQUIRE(dictCount == 1);
}

TEST_CASE("DeduplicateWithReferences")
{
    PdfMemDocument doc;
    
    // Create objects that reference each other
    auto& refObj = doc.GetObjects().CreateObject(PdfObject(42));
    
    PdfArray arr1;
    arr1.Add(refObj.GetIndirectReference());
    
    PdfArray arr2;
    arr2.Add(refObj.GetIndirectReference());
    
    auto& obj1 = doc.GetObjects().CreateObject(arr1);
    auto& obj2 = doc.GetObjects().CreateObject(arr2);
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // The arrays should have been deduplicated
    int arrayCount = 0;
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsArray())
            arrayCount++;
    }
    
    REQUIRE(arrayCount == 1);
}

TEST_CASE("DeduplicateAggressiveMode")
{
    PdfMemDocument doc;
    
    // Create a page to test with stream content
    auto& page = doc.GetPages().CreatePage(PdfPageSize::A4);
    
    // Create some content on the page
    auto& painter = page.GetPainter();
    painter.SetColor(0.5, 0.5, 0.5);
    painter.DrawRect(100, 100, 200, 200);
    painter.FinishDrawing();
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform aggressive deduplication
    doc.DeduplicateObjects(true);
    
    // Should not crash and should maintain document integrity
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize > 0);
    
    // Document should still be valid
    REQUIRE(doc.GetPages().GetCount() == 1);
}

TEST_CASE("DeduplicateNonAggressiveMode")
{
    PdfMemDocument doc;
    
    // Create a page
    auto& page = doc.GetPages().CreatePage(PdfPageSize::A4);
    
    // Create some content on the page
    auto& painter = page.GetPainter();
    painter.SetColor(0.5, 0.5, 0.5);
    painter.DrawRect(100, 100, 200, 200);
    painter.FinishDrawing();
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform non-aggressive deduplication
    doc.DeduplicateObjects(false);
    
    // Should not crash and should maintain document integrity
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize > 0);
    
    // Document should still be valid
    REQUIRE(doc.GetPages().GetCount() == 1);
}

TEST_CASE("DeduplicateComplexNestedObjects")
{
    PdfMemDocument doc;
    
    // Create complex nested objects
    PdfDictionary innerDict1;
    innerDict1.AddKey("inner", PdfObject(123));
    
    PdfDictionary innerDict2;
    innerDict2.AddKey("inner", PdfObject(123));
    
    PdfArray outerArr1;
    outerArr1.Add(innerDict1);
    outerArr1.Add(PdfObject(456));
    
    PdfArray outerArr2;
    outerArr2.Add(innerDict2);
    outerArr2.Add(PdfObject(456));
    
    auto& obj1 = doc.GetObjects().CreateObject(outerArr1);
    auto& obj2 = doc.GetObjects().CreateObject(outerArr2);
    
    unsigned initialSize = doc.GetObjects().GetSize();
    
    // Perform deduplication
    doc.DeduplicateObjects();
    
    // Should have reduced the number of objects
    unsigned finalSize = doc.GetObjects().GetSize();
    REQUIRE(finalSize < initialSize);
    
    // Should only have one outer array
    int outerArrayCount = 0;
    for (auto obj : doc.GetObjects())
    {
        if (obj->IsArray() && obj->GetArray().GetSize() == 2)
            outerArrayCount++;
    }
    
    REQUIRE(outerArrayCount == 1);
} 