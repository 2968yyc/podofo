/**
 * SPDX-FileCopyrightText: (C) 2024 AI Assistant
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <podofo/podofo.h>
#include <iostream>

using namespace std;
using namespace PoDoFo;

int main()
{
    try
    {
        PdfMemDocument doc;
        
        // Create a page
        auto& page = doc.GetPages().CreatePage(PdfPageSize::A4);
        
        // Create some duplicate objects
        auto& duplicateString1 = doc.GetObjects().CreateObject(PdfObject(PdfString("Duplicate String")));
        auto& duplicateString2 = doc.GetObjects().CreateObject(PdfObject(PdfString("Duplicate String")));
        
        auto& duplicateNumber1 = doc.GetObjects().CreateObject(PdfObject(42));
        auto& duplicateNumber2 = doc.GetObjects().CreateObject(PdfObject(42));
        auto& duplicateNumber3 = doc.GetObjects().CreateObject(PdfObject(42));
        
        // Create duplicate arrays
        PdfArray arr1;
        arr1.Add(PdfObject(1));
        arr1.Add(PdfObject(2));
        arr1.Add(PdfObject(3));
        
        PdfArray arr2;
        arr2.Add(PdfObject(1));
        arr2.Add(PdfObject(2));
        arr2.Add(PdfObject(3));
        
        auto& duplicateArray1 = doc.GetObjects().CreateObject(arr1);
        auto& duplicateArray2 = doc.GetObjects().CreateObject(arr2);
        
        // Create duplicate dictionaries
        PdfDictionary dict1;
        dict1.AddKey("Key1", PdfObject(100));
        dict1.AddKey("Key2", PdfObject(PdfString("Value")));
        dict1.AddKey("Key3", duplicateNumber1.GetIndirectReference());
        
        PdfDictionary dict2;
        dict2.AddKey("Key1", PdfObject(100));
        dict2.AddKey("Key2", PdfObject(PdfString("Value")));
        dict2.AddKey("Key3", duplicateNumber2.GetIndirectReference());
        
        auto& duplicateDict1 = doc.GetObjects().CreateObject(dict1);
        auto& duplicateDict2 = doc.GetObjects().CreateObject(dict2);
        
        // Create some content on the page that references these objects
        auto& painter = page.GetPainter();
        painter.SetColor(0.5, 0.5, 0.5);
        painter.DrawRect(100, 100, 200, 200);
        
        // Add some text
        painter.SetColor(0, 0, 0);
        painter.DrawText("Test PDF with duplicate objects", 100, 300);
        painter.FinishDrawing();
        
        // Save the document
        doc.Save("test-with-duplicates.pdf");
        
        cout << "Created test PDF with " << doc.GetObjects().GetSize() << " objects." << endl;
        cout << "File saved as: test-with-duplicates.pdf" << endl;
        
        return 0;
    }
    catch (const PdfError& e)
    {
        cerr << "PoDoFo Error: " << e.what() << endl;
        return 1;
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
} 