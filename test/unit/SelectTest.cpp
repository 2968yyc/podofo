/**
 * SPDX-FileCopyrightText: (C) 2024 AI Assistant
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <PdfTest.h>

using namespace std;
using namespace PoDoFo;

TEST_CASE("SelectEmptyDocument")
{
    PdfMemDocument doc;
    
    // Test with empty page numbers - should keep document unchanged
    vector<unsigned> pageNumbers;
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 0);
    
    // Test with invalid page numbers - should clear document
    pageNumbers = { 0, 1, 2 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 0);
}

TEST_CASE("SelectSinglePage")
{
    PdfMemDocument doc;
    
    // Create a single page
    auto& page = doc.GetPages().CreatePage(PdfPageSize::A4);
    REQUIRE(doc.GetPages().GetCount() == 1);
    
    // Select the same page - should keep it
    vector<unsigned> pageNumbers = { 0 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 1);
    
    // Select with invalid page number - should clear document
    pageNumbers = { 1 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 0);
}

TEST_CASE("SelectMultiplePages")
{
    PdfMemDocument doc;
    
    // Create multiple pages
    for (int i = 0; i < 5; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 5);
    
    // Select pages in reverse order
    vector<unsigned> pageNumbers = { 4, 3, 2, 1, 0 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 5);
    
    // Select only first and last pages
    pageNumbers = { 0, 4 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 2);
    
    // Select with duplicate page numbers
    pageNumbers = { 0, 0, 1, 1 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 4);
}

TEST_CASE("SelectWithInvalidPageNumbers")
{
    PdfMemDocument doc;
    
    // Create 3 pages
    for (int i = 0; i < 3; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 3);
    
    // Select with some invalid page numbers - should filter them out
    vector<unsigned> pageNumbers = { 0, 5, 1, 10, 2 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 3);
    
    // Select with all invalid page numbers - should clear document
    pageNumbers = { 5, 10, 15 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 0);
}

TEST_CASE("SelectPartialPages")
{
    PdfMemDocument doc;
    
    // Create 10 pages
    for (int i = 0; i < 10; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 10);
    
    // Select only even pages
    vector<unsigned> pageNumbers = { 0, 2, 4, 6, 8 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 5);
    
    // Select only odd pages
    pageNumbers = { 1, 3, 5, 7, 9 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 5);
}

TEST_CASE("SelectWithMixedValidInvalid")
{
    PdfMemDocument doc;
    
    // Create 5 pages
    for (int i = 0; i < 5; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 5);
    
    // Select with mixed valid and invalid page numbers
    vector<unsigned> pageNumbers = { 0, 10, 1, 20, 2, 30, 3, 40, 4 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 5);
    
    // Verify the order is maintained
    // Note: We can't easily verify the content, but we can verify the count
    // and that the operation completes without errors
}

TEST_CASE("SelectComplexReordering")
{
    PdfMemDocument doc;
    
    // Create 8 pages
    for (int i = 0; i < 8; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 8);
    
    // Complex reordering: move pages around
    vector<unsigned> pageNumbers = { 7, 0, 6, 1, 5, 2, 4, 3 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 8);
    
    // Select in groups
    pageNumbers = { 0, 1, 2, 3, 7, 6, 5, 4 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 8);
}

TEST_CASE("SelectWithDuplicates")
{
    PdfMemDocument doc;
    
    // Create 3 pages
    for (int i = 0; i < 3; i++)
    {
        doc.GetPages().CreatePage(PdfPageSize::A4);
    }
    REQUIRE(doc.GetPages().GetCount() == 3);
    
    // Select with duplicates - should include each page multiple times
    vector<unsigned> pageNumbers = { 0, 0, 1, 1, 1, 2, 2 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 7);
    
    // Select same page multiple times
    pageNumbers = { 1, 1, 1, 1, 1 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 5);
}

TEST_CASE("SelectEdgeCases")
{
    PdfMemDocument doc;
    
    // Create 1 page
    doc.GetPages().CreatePage(PdfPageSize::A4);
    REQUIRE(doc.GetPages().GetCount() == 1);
    
    // Select with empty vector after having pages
    vector<unsigned> pageNumbers;
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 1); // Should remain unchanged
    
    // Select with out-of-range numbers
    pageNumbers = { 1, 2, 3 };
    doc.Select(pageNumbers);
    REQUIRE(doc.GetPages().GetCount() == 0); // Should clear document
} 