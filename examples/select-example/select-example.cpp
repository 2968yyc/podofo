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
    cout << "Usage: select-example <input.pdf> <output.pdf> [page_numbers...]" << endl;
    cout << "Example: select-example input.pdf output.pdf 0 2 1 3" << endl;
    cout << "This will reorder pages: page 0, page 2, page 1, page 3" << endl;
    cout << "Page numbers are 0-based." << endl;
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
        
        // Load the PDF document
        PdfMemDocument doc;
        doc.Load(inputFile);
        
        cout << "Original document has " << doc.GetPages().GetCount() << " pages." << endl;
        
        // Parse page numbers from command line arguments
        vector<unsigned> pageNumbers;
        for (int i = 3; i < argc; i++)
        {
            try
            {
                unsigned pageNum = static_cast<unsigned>(stoul(argv[i]));
                pageNumbers.push_back(pageNum);
            }
            catch (const exception&)
            {
                cerr << "Warning: Invalid page number '" << argv[i] << "', ignoring." << endl;
            }
        }
        
        // If no page numbers provided, keep all pages in current order
        if (pageNumbers.empty())
        {
            cout << "No page numbers provided, keeping all pages in current order." << endl;
        }
        else
        {
            cout << "Selecting pages: ";
            for (size_t i = 0; i < pageNumbers.size(); i++)
            {
                cout << pageNumbers[i];
                if (i < pageNumbers.size() - 1)
                    cout << ", ";
            }
            cout << endl;
            
            // Apply the page selection
            doc.Select(pageNumbers);
        }
        
        cout << "Document now has " << doc.GetPages().GetCount() << " pages." << endl;
        
        // Save the modified document
        doc.Save(outputFile);
        
        cout << "Modified document saved to: " << outputFile << endl;
        
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