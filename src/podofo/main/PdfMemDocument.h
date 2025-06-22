/**
 * SPDX-FileCopyrightText: (C) 2006 Dominik Seichter <domseichter@web.de>
 * SPDX-FileCopyrightText: (C) 2020 Francesco Pretto <ceztko@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef PDF_MEM_DOCUMENT_H
#define PDF_MEM_DOCUMENT_H

#include <podofo/auxiliary/InputDevice.h>
#include <podofo/auxiliary/OutputDevice.h>

#include "PdfDocument.h"
#include "PdfEncryptSession.h"

namespace PoDoFo {

class PdfParser;
class PdfEncryptSession;

/** PdfMemDocument is the core class for reading and manipulating
 *  PDF files and writing them back to disk.
 *
 *  PdfMemDocument was designed to allow easy access to the object
 *  structure of a PDF file.
 *
 *  PdfMemDocument should be used whenever you want to change
 *  the object structure of a PDF file.
 *
 *  When you are only creating PDF files, please use PdfStreamedDocument
 *  which is usually faster for creating PDFs.
 *
 *  \see PdfDocument
 *  \see PdfStreamedDocument
 *  \see PdfParser
 */
class PODOFO_API PdfMemDocument final : public PdfDocument
{
    PODOFO_PRIVATE_FRIEND(class PdfWriter);

public:
    /** Construct a new PdfMemDocument
     */
    PdfMemDocument();

    PdfMemDocument(std::shared_ptr<InputStreamDevice> device, const std::string_view& password = { });

    /** Construct a copy of the given document
     */
    PdfMemDocument(const PdfMemDocument& rhs);

    /** Load a PdfMemDocument from a file
     *
     *  \param filename filename of the file which is going to be parsed/opened
     *
     *  When the bForUpdate is set to true, the filename is copied
     *  for later use by WriteUpdate.
     *
     *  \see WriteUpdate, LoadFromBuffer, LoadFromDevice
     */
    void Load(const std::string_view& filename, const std::string_view& password = { });

    /** Load a PdfMemDocument from a buffer in memory
     *
     *  \param buffer a memory area containing the PDF data
     *
     *  \see WriteUpdate, Load, LoadFromDevice
     */
    void LoadFromBuffer(const bufferview& buffer, const std::string_view& password = { });

    /** Load a PdfMemDocument from a PdfRefCountedInputDevice
     *
     *  \param device the input device containing the PDF
     *
     *  \see WriteUpdate, Load, LoadFromBuffer
     */
    void Load(std::shared_ptr<InputStreamDevice> device, const std::string_view& password = { });

    /** Save the complete document to a file
     *
     *  \param filename filename of the document
     *
     *  \see Save, SaveUpdate
     *
     *  This is an overloaded member function for your convenience.
     */
    void Save(const std::string_view& filename, PdfSaveOptions opts = PdfSaveOptions::None);

    /** Save the complete document to an output device
     *
     *  \param device write to this output device
     *
     *  \see SaveUpdate
     */
    void Save(OutputStreamDevice& device, PdfSaveOptions opts = PdfSaveOptions::None);

    /** Save the document changes to a file
     *
     *  \param filename filename of the document
     *
     *  Writes the document changes to a file as an incremental update.
     *  The document should be loaded with bForUpdate = true, otherwise
     *  an exception is thrown.
     *
     *  Beware when overwriting existing files. Plain file overwrite is allowed
     *  only if the document was loaded with the same filename (and the same overloaded
     *  function), otherwise the destination file cannot be the same as the source file,
     *  because the destination file is truncated first and only then the source file
     *  content is copied into it.
     *
     *  \see Save, SaveUpdate
     *
     *  This is an overloaded member function for your convenience.
     */
    void SaveUpdate(const std::string_view& filename, PdfSaveOptions opts = PdfSaveOptions::None);

    /** Save the document changes to an output device
     *
     *  \param device write to this output device
     *
     *  Writes the document changes to the output device as an incremental update.
     *  The document should be loaded with bForUpdate = true, otherwise
     *  an exception is thrown.
     *
     *  \see Save, SaveUpdate
     */
    void SaveUpdate(OutputStreamDevice& device, PdfSaveOptions opts = PdfSaveOptions::None);

    /** Select and reorder pages in the document
     *
     *  \param pageNumbers vector of page numbers (0-based) to select and reorder
     *                     If empty, all pages are selected in their current order
     *                     If a page number is out of range, it will be ignored
     *                     Duplicate page numbers are allowed and will be included
     *
     *  This method reorders the pages in the document according to the provided
     *  page numbers. Pages not included in the pageNumbers vector will be removed.
     *  The order of pages in the resulting document will match the order in pageNumbers.
     *
     *  \see PyMuPDF Document.select() for similar functionality
     */
    void Select(const std::vector<unsigned>& pageNumbers);

    /** Deduplicate objects in the document
     *
     *  This method performs object deduplication similar to mutool clean -gggg.
     *  It identifies duplicate objects and merges them, updating all references
     *  to point to a single instance of each unique object.
     *
     *  The deduplication process:
     *  1. Identifies objects with identical content
     *  2. Keeps one instance of each unique object
     *  3. Updates all references to point to the kept objects
     *  4. Removes duplicate objects
     *  5. Performs garbage collection to clean up unreferenced objects
     *
     *  \param aggressive if true, performs more aggressive deduplication including
     *                    stream content comparison; if false, only compares object structure
     *
     *  \see mutool clean -gggg for similar functionality
     */
    void DeduplicateObjects(bool aggressive = true);

    /** Encrypt the document during writing.
     *
     *  \param userPassword the user password (if empty the user does not have
     *                      to enter a password to open the document)
     *  \param ownerPassword the owner password
     *  \param protection several PdfPermissions values or'ed together to set
     *                    the users permissions for this document
     *  \param algorithm the revision of the encryption algorithm to be used
     *  \param keyLength the length of the encryption key ranging from 40 to 256 bits
     *                    (only used if algorithm >= PdfEncryptAlgorithm::RC4V2)
     *
     *  \see PdfEncrypt
     */
    void SetEncrypted(const std::string_view& userPassword, const std::string_view& ownerPassword,
        PdfPermissions protection = PdfPermissions::Default,
        PdfEncryptionAlgorithm algorithm = PdfEncryptionAlgorithm::AESV3R6,
        PdfKeyLength keyLength = PdfKeyLength::Unknown);

    /** Encrypt the document during writing using a PdfEncrypt object
     *
     *  \param encrypt an encryption object that will be owned by PdfMemDocument
     */
    void SetEncrypt(std::unique_ptr<PdfEncrypt>&& encrypt);

    const PdfEncrypt* GetEncrypt() const override;

protected:
    /** Set the PDF Version of the document. Has to be called before Write() to
     *  have an effect.
     *  \param version  version of the pdf document
     */
    void SetPdfVersion(PdfVersion version) override;

    PdfVersion GetPdfVersion() const override;

private:
    PdfMemDocument(bool empty);

private:
    void loadFromDevice(std::shared_ptr<InputStreamDevice>&& device, const std::string_view& password);

    /** Internal method to load all objects from a PdfParser object.
     *  The objects will be removed from the parser and are now
     *  owned by the PdfMemDocument.
     */
    void initFromParser(PdfParser& parser);

    /** Clear all variables that have internal memory usage
      */
    void clear() override;

    void reset() override;

    void beforeWrite(PdfSaveOptions options);

    /** Get a string representation of object content for deduplication
     *  \param obj the object to get content for
     *  \param aggressive if true, includes stream content in comparison
     *  \returns string representation of object content
     */
    std::string getObjectContent(const PdfObject& obj, bool aggressive);

    /** Update all object references in the document according to replacement map
     *  \param replacementMap map of old references to new references
     */
    void updateObjectReferences(const std::unordered_map<PdfReference, PdfReference>& replacementMap);

    /** Recursively update object references in a single object
     *  \param obj the object to update references in
     *  \param replacementMap map of old references to new references
     */
    void updateObjectReferencesRecursive(PdfObject& obj, const std::unordered_map<PdfReference, PdfReference>& replacementMap);

private:
    PdfMemDocument& operator=(const PdfMemDocument&) = delete;

private:
    PdfVersion m_Version;
    PdfVersion m_InitialVersion;
    bool m_HasXRefStream;
    int64_t m_PrevXRefOffset;
    std::unique_ptr<PdfEncryptSession> m_Encrypt;
    std::shared_ptr<InputStreamDevice> m_device;
};

};


#endif	// PDF_MEM_DOCUMENT_H
