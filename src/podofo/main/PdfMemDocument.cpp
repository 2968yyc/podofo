/**
 * SPDX-FileCopyrightText: (C) 2006 Dominik Seichter <domseichter@web.de>
 * SPDX-FileCopyrightText: (C) 2020 Francesco Pretto <ceztko@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <podofo/private/PdfDeclarationsPrivate.h>
#include "PdfMemDocument.h"

#include <podofo/auxiliary/StreamDevice.h>
#include <podofo/private/PdfWriter.h>
#include <podofo/private/PdfParser.h>
#include "PdfXObjectForm.h"
#include "PdfPage.h"
#include "PdfResources.h"
#include "PdfContents.h"
#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfObject.h"
#include "PdfReference.h"
#include "PdfObjectStream.h"

#include "PdfCommon.h"

#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace PoDoFo;

PdfMemDocument::PdfMemDocument()
    : PdfMemDocument(false) { }

PdfMemDocument::PdfMemDocument(bool empty) :
    PdfDocument(empty),
    m_Version(PdfVersionDefault),
    m_InitialVersion(PdfVersionDefault),
    m_HasXRefStream(false),
    m_PrevXRefOffset(-1)
{
}

PdfMemDocument::PdfMemDocument(shared_ptr<InputStreamDevice> device, const string_view& password)
    : PdfMemDocument(true)
{
    if (device == nullptr)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    loadFromDevice(std::move(device), password);
}

PdfMemDocument::PdfMemDocument(const PdfMemDocument& rhs) :
    PdfDocument(rhs),
    m_Version(rhs.m_Version),
    m_InitialVersion(rhs.m_InitialVersion),
    m_HasXRefStream(rhs.m_HasXRefStream),
    m_PrevXRefOffset(rhs.m_PrevXRefOffset)
{
    // Do a full copy of the encrypt session
    if (rhs.m_Encrypt != nullptr)
        m_Encrypt.reset(new PdfEncryptSession(rhs.m_Encrypt->GetEncrypt(), rhs.m_Encrypt->GetContext()));
}

void PdfMemDocument::clear()
{
    // NOTE: Here we clear only variables that have memory
    // usage. The other variables get initialized by parsing or reset
    m_Encrypt = nullptr;
    m_device = nullptr;
}

void PdfMemDocument::reset()
{
    m_Version = PdfVersionDefault;
    m_InitialVersion = PdfVersionDefault;
    m_HasXRefStream = false;
    m_PrevXRefOffset = -1;
}

void PdfMemDocument::initFromParser(PdfParser& parser)
{
    m_Version = parser.GetPdfVersion();
    m_InitialVersion = m_Version;
    m_HasXRefStream = parser.HasXRefStream();
    m_PrevXRefOffset = parser.GetXRefOffset();
    this->SetTrailer(parser.TakeTrailer());

    if (PdfCommon::IsLoggingSeverityEnabled(PdfLogSeverity::Debug))
    {
        auto debug = GetTrailer().GetObject().ToString();
        debug.push_back('\n');
        PoDoFo::LogMessage(PdfLogSeverity::Debug, debug);
    }

    auto encrypt = parser.GetEncrypt();
    if (encrypt != nullptr)
        m_Encrypt.reset(new PdfEncryptSession(*encrypt));

    Init();
}

void PdfMemDocument::Load(const string_view& filename, const string_view& password)
{
    if (filename.length() == 0)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    auto device = std::make_shared<FileStreamDevice>(filename);
    Load(device, password);
}

void PdfMemDocument::LoadFromBuffer(const bufferview& buffer, const string_view& password)
{
    if (buffer.size() == 0)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    auto device = std::make_shared<SpanStreamDevice>(buffer);
    Load(device, password);
}

void PdfMemDocument::Load(shared_ptr<InputStreamDevice> device, const string_view& password)
{
    if (device == nullptr)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    this->Clear();
    loadFromDevice(std::move(device), password);
}

void PdfMemDocument::loadFromDevice(shared_ptr<InputStreamDevice>&& device, const string_view& password)
{
    m_device = std::move(device);

    // Call parse file instead of using the constructor
    // so that m_Parser is initialized for encrypted documents
    PdfParser parser(PdfDocument::GetObjects());
    parser.SetPassword(password);
    parser.Parse(*m_device, true);
    initFromParser(parser);
}

void PdfMemDocument::Save(const string_view& filename, PdfSaveOptions options)
{
    FileStreamDevice device(filename, FileMode::Create);
    this->Save(device, options);
}

void PdfMemDocument::Save(OutputStreamDevice& device, PdfSaveOptions opts)
{
    beforeWrite(opts);

    PdfWriter writer(this->GetObjects(), this->GetTrailer().GetObject());
    writer.SetPdfVersion(GetMetadata().GetPdfVersion());
    writer.SetPdfALevel(GetMetadata().GetPdfALevel());
    writer.SetSaveOptions(opts);

    if (m_Encrypt != nullptr)
        writer.SetEncrypt(*m_Encrypt);

    try
    {
        writer.Write(device);
    }
    catch (PdfError& e)
    {
        PODOFO_PUSH_FRAME(e);
        throw;
    }
}

void PdfMemDocument::SaveUpdate(const string_view& filename, PdfSaveOptions opts)
{
    FileStreamDevice device(filename, FileMode::Append);
    this->SaveUpdate(device, opts);
}

void PdfMemDocument::SaveUpdate(OutputStreamDevice& device, PdfSaveOptions opts)
{
    beforeWrite(opts);

    PdfWriter writer(this->GetObjects(), this->GetTrailer().GetObject());
    writer.SetPdfVersion(GetMetadata().GetPdfVersion());
    writer.SetPdfALevel(GetMetadata().GetPdfALevel());
    writer.SetSaveOptions(opts);
    writer.SetPrevXRefOffset(m_PrevXRefOffset);
    writer.SetUseXRefStream(m_HasXRefStream);
    writer.SetIncrementalUpdate(false);

    if (m_Encrypt != nullptr)
        writer.SetEncrypt(*m_Encrypt);

    if (m_InitialVersion < this->GetPdfVersion())
    {
        if (this->GetPdfVersion() < PdfVersion::V1_0 || this->GetPdfVersion() > PdfVersion::V1_7)
            PODOFO_RAISE_ERROR(PdfErrorCode::ValueOutOfRange);

        GetCatalog().GetDictionary().AddKey("Version"_n, PoDoFo::GetPdfVersionName(GetPdfVersion()));
    }

    try
    {
        device.Seek(0, SeekDirection::End);
        writer.Write(device);
    }
    catch (PdfError& e)
    {
        PODOFO_PUSH_FRAME(e);
        throw;
    }
}

void PdfMemDocument::beforeWrite(PdfSaveOptions opts)
{
    if ((opts & PdfSaveOptions::NoMetadataUpdate) ==
        PdfSaveOptions::None)
    {
        GetMetadata().SetModifyDate(PdfDate::LocalNow());
        (void)GetMetadata().TrySyncXMPMetadata();
    }

    GetFonts().EmbedFonts();

    // After we are done with all operations on objects,
    // we can collect garbage
    if ((opts & PdfSaveOptions::NoCollectGarbage) ==
        PdfSaveOptions::None)
    {
        CollectGarbage();
    }
}

void PdfMemDocument::SetEncrypted(const string_view& userPassword, const string_view& ownerPassword,
    PdfPermissions protection, PdfEncryptionAlgorithm algorithm,
    PdfKeyLength keyLength)
{
    m_Encrypt.reset(new PdfEncryptSession(PdfEncrypt::Create(userPassword, ownerPassword, protection, algorithm, keyLength)));
}

void PdfMemDocument::SetEncrypt(unique_ptr<PdfEncrypt>&& encrypt)
{
    if (encrypt == nullptr)
        m_Encrypt = nullptr;
    else
        m_Encrypt.reset(new PdfEncryptSession(std::move(encrypt)));
}

const PdfEncrypt* PdfMemDocument::GetEncrypt() const
{
    if (m_Encrypt == nullptr)
        return nullptr;

    return &m_Encrypt->GetEncrypt();
}

void PdfMemDocument::SetPdfVersion(PdfVersion version)
{
    m_Version = version;
}

PdfVersion PdfMemDocument::GetPdfVersion() const
{
    return m_Version;
}

void PdfMemDocument::Select(const std::vector<unsigned>& pageNumbers)
{
    auto& pages = this->GetPages();
    unsigned totalPages = pages.GetCount();
    
    // If no page numbers provided, keep all pages in current order
    if (pageNumbers.empty())
        return;
    
    // Validate page numbers and filter out invalid ones
    std::vector<unsigned> validPageNumbers;
    for (unsigned pageNum : pageNumbers)
    {
        if (pageNum < totalPages)
            validPageNumbers.push_back(pageNum);
    }
    
    // If no valid page numbers, clear all pages
    if (validPageNumbers.empty())
    {
        while (pages.GetCount() > 0)
            pages.RemovePageAt(0);
        return;
    }
    
    // Create a temporary document to store the selected pages
    PdfMemDocument tempDoc;
    
    // Copy selected pages to temporary document in the specified order
    for (unsigned pageNum : validPageNumbers)
    {
        // Get the page from the original document
        PdfPage& originalPage = pages.GetPageAt(pageNum);
        
        // Create a new page in the temporary document with the same size
        PdfPage& newPage = tempDoc.GetPages().CreatePage(originalPage.GetMediaBox());
        
        // Copy the page content by creating an XObject from the original page
        auto xobj = this->CreateXObjectForm(originalPage.GetMediaBox());
        this->FillXObjectFromPage(*xobj, originalPage, false);
        
        // Add the XObject to the new page
        newPage.GetResources().AddResource("XObject"_n, "Page"_n, xobj->GetObject());
        
        // Create a content stream that shows the XObject
        auto& contents = newPage.GetOrCreateContents();
        auto& stream = contents.GetOrCreateStream();
        auto output = stream.GetOutputStream({ PdfFilterType::FlateDecode });
        
        // Write the content stream to display the XObject
        std::string contentStr = "q\n";
        contentStr += "1 0 0 1 0 0 cm\n";  // Identity matrix
        contentStr += "/Page Do\n";         // Display the XObject
        contentStr += "Q\n";                // Restore graphics state
        
        output.Write(contentStr);
    }
    
    // Clear the original document's pages
    while (pages.GetCount() > 0)
        pages.RemovePageAt(0);
    
    // Copy all pages from temporary document back to original document
    this->AppendDocumentPages(tempDoc);
}

void PdfMemDocument::DeduplicateObjects(bool aggressive)
{
    auto& objects = this->GetObjects();
    
    // Step 1: Build a map of object content to object references
    std::unordered_map<std::string, std::vector<PdfReference>> contentMap;
    std::unordered_map<PdfReference, std::string> objectContent;
    
    for (auto obj : objects)
    {
        if (obj == nullptr)
            continue;
            
        std::string content = getObjectContent(*obj, aggressive);
        contentMap[content].push_back(obj->GetIndirectReference());
        objectContent[obj->GetIndirectReference()] = content;
    }
    
    // Step 2: Identify duplicates and create replacement map
    std::unordered_map<PdfReference, PdfReference> replacementMap;
    std::unordered_set<PdfReference> objectsToRemove;
    
    for (const auto& pair : contentMap)
    {
        const auto& refs = pair.second;
        if (refs.size() > 1)
        {
            // Keep the first reference, replace all others
            PdfReference keepRef = refs[0];
            for (size_t i = 1; i < refs.size(); i++)
            {
                replacementMap[refs[i]] = keepRef;
                objectsToRemove.insert(refs[i]);
            }
        }
    }
    
    // Step 3: Update all references in the document
    updateObjectReferences(replacementMap);
    
    // Step 4: Remove duplicate objects
    for (const auto& ref : objectsToRemove)
    {
        objects.RemoveObject(ref);
    }
    
    // Step 5: Perform garbage collection
    this->CollectGarbage();
}

std::string PdfMemDocument::getObjectContent(const PdfObject& obj, bool aggressive)
{
    std::stringstream ss;
    
    switch (obj.GetDataType())
    {
        case PdfDataType::Null:
            ss << "null";
            break;
            
        case PdfDataType::Boolean:
            ss << "bool:" << (obj.GetBool() ? "true" : "false");
            break;
            
        case PdfDataType::Number:
            if (obj.IsNumber())
                ss << "num:" << obj.GetNumber();
            else
                ss << "int:" << obj.GetInt64();
            break;
            
        case PdfDataType::Real:
            ss << "real:" << obj.GetReal();
            break;
            
        case PdfDataType::String:
            ss << "str:" << obj.GetString().GetString();
            break;
            
        case PdfDataType::Name:
            ss << "name:" << obj.GetName().GetString();
            break;
            
        case PdfDataType::Array:
        {
            ss << "array:[";
            const auto& arr = obj.GetArray();
            for (size_t i = 0; i < arr.GetSize(); i++)
            {
                if (i > 0) ss << ",";
                ss << getObjectContent(arr[i], aggressive);
            }
            ss << "]";
            break;
        }
        
        case PdfDataType::Dictionary:
        {
            ss << "dict:{";
            const auto& dict = obj.GetDictionary();
            bool first = true;
            for (const auto& pair : dict)
            {
                if (!first) ss << ",";
                ss << pair.first.GetString() << ":" << getObjectContent(pair.second, aggressive);
                first = false;
            }
            ss << "}";
            
            // Include stream content if aggressive mode is enabled
            if (aggressive && obj.HasStream())
            {
                ss << "stream:";
                auto& stream = obj.GetOrCreateStream();
                charbuff buffer;
                stream.CopyTo(buffer);
                ss << std::string(buffer.data(), buffer.size());
            }
            break;
        }
        
        case PdfDataType::Reference:
            // For references, we include the reference itself in the content
            // This ensures that identical references are considered the same
            ss << "ref:" << obj.GetReference().ObjectNumber() << ":" << obj.GetReference().GenerationNumber();
            break;
            
        default:
            ss << "unknown";
            break;
    }
    
    return ss.str();
}

void PdfMemDocument::updateObjectReferences(const std::unordered_map<PdfReference, PdfReference>& replacementMap)
{
    if (replacementMap.empty())
        return;
        
    auto& objects = this->GetObjects();
    
    for (auto obj : objects)
    {
        if (obj == nullptr)
            continue;
            
        updateObjectReferencesRecursive(*obj, replacementMap);
    }
}

void PdfMemDocument::updateObjectReferencesRecursive(PdfObject& obj, const std::unordered_map<PdfReference, PdfReference>& replacementMap)
{
    switch (obj.GetDataType())
    {
        case PdfDataType::Reference:
        {
            auto it = replacementMap.find(obj.GetReference());
            if (it != replacementMap.end())
            {
                obj = PdfObject(it->second);
            }
            break;
        }
        
        case PdfDataType::Array:
        {
            auto& arr = obj.GetArrayUnsafe();
            for (auto& child : arr)
            {
                updateObjectReferencesRecursive(child, replacementMap);
            }
            break;
        }
        
        case PdfDataType::Dictionary:
        {
            auto& dict = obj.GetDictionaryUnsafe();
            for (auto& pair : dict)
            {
                updateObjectReferencesRecursive(pair.second, replacementMap);
            }
            break;
        }
        
        default:
            // No references to update in other data types
            break;
    }
}
