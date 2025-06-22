/**
 * SPDX-FileCopyrightText: (C) 2024 AI Assistant
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <podofo/podofo.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace PoDoFo;

void printUsage()
{
    cout << "Usage: deduplicate-example <input.pdf> <output.pdf> [--aggressive]" << endl;
    cout << "Example: deduplicate-example input.pdf output.pdf --aggressive" << endl;
    cout << "This will deduplicate objects in the PDF, similar to mutool clean -gggg" << endl;
    cout << "Options:" << endl;
    cout << "  --aggressive  Perform aggressive deduplication including stream content" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    try
    {
        string inputFile = argv[1];
        string outputFile = argv[2];
        bool aggressive = false;
        
        // Parse command line arguments
        for (int i = 3; i < argc; i++)
        {
            string arg = argv[i];
            if (arg == "--aggressive")
            {
                aggressive = true;
            }
            else
            {
                cerr << "Unknown argument: " << arg << endl;
                printUsage();
                return 1;
            }
        }
        
        // Load the PDF document
        PdfMemDocument doc;
        doc.Load(inputFile);
        
        cout << "Original document has " << doc.GetObjects().GetSize() << " objects." << endl;
        
        // Perform object deduplication
        cout << "Performing " << (aggressive ? "aggressive" : "standard") << " deduplication..." << endl;
        doc.DeduplicateObjects(aggressive);
        
        cout << "Document now has " << doc.GetObjects().GetSize() << " objects." << endl;
        
        // Save the deduplicated document
        doc.Save(outputFile);
        
        cout << "Deduplicated document saved to: " << outputFile << endl;
        
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