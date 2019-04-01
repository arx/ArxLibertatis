/*
 * Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/zip/ZipReader.h"

#include <zlib.h>

#include "io/fs/FileStream.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"

namespace zip {

// Zip File implementation based on
// http://www.pkware.com/documents/casestudies/APPNOTE.TXT

// This parser is intentionally stricter !
// files must start with a local file header !

#pragma pack(push, 1)

// 4.4.4 general purpose bit flag
struct GeneralPurposeBitFlag {
	u8 file_is_encrypted: 1;
	u8 method_param_bits: 2;
	u8 data_descriptor_enabled: 1;
	u8 enhanced_deflating: 1;
	u8 compressed_patched_data: 1;
	u8 strong_encryption: 1;
	u8 unused_7: 1;
	u8 unused_8: 1;
	u8 unused_9: 1;
	u8 unused_10: 1;
	u8 language_encoding_flag: 1; // Bit 11: Language encoding flag (EFS)
	u8 reserved_12: 1;
	u8 masked_local_header: 1;
	u8 reserved_14: 1;
	u8 reserved_15: 1;
};
ARX_STATIC_ASSERT(sizeof(GeneralPurposeBitFlag) == 2, "GeneralPurposeBitFlag not 2 bytes");

// 4.4.5 compression method
enum CompressionMethod {
	CompressionMethod_Store = 0,
	CompressionMethod_Deflate = 8
};

// 4.3.7  Local file header
static const u32 expected_local_file_header_signature = 0x04034b50;
struct LocalFileHeader {
	u32 local_file_header_signature;
	u16 version_needed_to_extract;
	GeneralPurposeBitFlag general_purpose_bit_flag;
	u16 compression_method;
	u16 last_mod_file_time;
	u16 last_mod_file_date;
	u32 crc_32;
	u32 compressed_size;
	u32 uncompressed_size;
	u16 filename_length;
	u16 extra_field_length;
};
ARX_STATIC_ASSERT(sizeof(LocalFileHeader) == 30, "LocalFileHeader not 30 bytes");

struct LocalFileHeader2 {
	std::string file_name;
	std::string extra_field;
};

// 4.3.9  Data descriptor
struct DataDescriptor {
	u32 crc_32;
	u32 compressed_size;
	u32 uncompressed_size;
};

// 4.3.11  Archive extra data record
struct ArchiveExtraData {
	u32 archive_extra_data_signature;
	u32 extra_field_length;
};

// 4.3.12  Central directory structure:
static const u32 expected_central_file_header_signature = 0x02014b50;
struct CentralDirectoryHeader {
	u32 central_file_header_signature;
	u16 version_made_by;
	u16 version_needed_to_extract;
	GeneralPurposeBitFlag general_purpose_bit_flag;
	u16 compression_method;
	u16 last_mod_file_time;
	u16 last_mod_file_date;
	u32 crc_32;
	u32 compressed_size;
	u32 uncompressed_size;
	u16 file_name_length;
	u16 extra_field_length;
	u16 file_comment_length;
	u16 disk_number_start;
	u16 internal_file_attributes;
	u32 external_file_attributes;
	u32 relative_offset_of_local_header;
};
ARX_STATIC_ASSERT(sizeof(CentralDirectoryHeader) == 46, "LocalFileHeader not 46 bytes");

struct CentralDirectoryHeader2 {
	std::string file_name;
	std::string extra_field;
	std::string file_comment;
};

// 4.3.16  End of central directory record
static const u32 expected_end_of_central_dir_signature = 0x06054b50;
struct EndOfCentralDirectoryRecord {
	u32 end_of_central_dir_signature;
	u16 number_of_this_disk;
	u16 number_of_the_disk_with_the_start_of_the_central_directory;
	u16 total_number_of_entries_in_the_central_directory_on_this_disk;
	u16 total_number_of_entries_in_the_central_directory;
	u32 size_of_the_central_directory;
	u32 foobar;
	u16 zip_file_comment_length;
};
ARX_STATIC_ASSERT(sizeof(EndOfCentralDirectoryRecord) == 22, "EndOfCentralDirectoryRecord not 22 bytes");

struct EndOfCentralDirectoryRecord2 {
	std::string zip_file_comment;
};

#pragma pack(pop)

class ZipFileReader {
	
	size_t m_fileSize;
	
	u32 peek_u32(fs::ifstream & ifs) {
		u32 val;
		ifs.read((char *)&val, sizeof(u32));
		ifs.seekg(-sizeof(u32), ifs.cur);
		return val;
	}
	
	void readString(fs::ifstream & ifs, std::string & string, size_t bytes) {
		string.resize(bytes);
		ifs.read(&string[0], bytes);
	}
	
	template<typename T>
	void readStruct(fs::ifstream & ifs, T & val) {
		ifs.read((char*)&val, sizeof(T));
	}
	
	void readLocalFileHeader(fs::ifstream & ifs) {
		
		zip::LocalFileHeader header;
		readStruct(ifs, header);

		LogInfo << "Zip entry filename_length " << header.filename_length;
		LogInfo << "Zip entry extra_field_length " << header.extra_field_length;
		
		if(header.local_file_header_signature != zip::expected_local_file_header_signature) {
			throw ZipFileException("Zip Entry: local_file_header_signature mismatch");
		}
		
		zip::LocalFileHeader2 header2;
		readString(ifs, header2.file_name, header.filename_length);
		readString(ifs, header2.extra_field, header.extra_field_length);
		
		LogInfo << "Zip Entry: filename " << header2.file_name;
		
		switch(header.compression_method) {
			case zip::CompressionMethod_Store: {
				if(header.compressed_size != header.uncompressed_size) {
					LogWarning << "Compressed and uncompressed size should be equal with compression method Store";
				}
				
				if(header.compressed_size != 0) {
					size_t offset = ifs.tellg();
					size_t size = header.compressed_size;
					m_callback.zipEntry(header2.file_name, offset, size);
				}
				break;
			}
			case zip::CompressionMethod_Deflate: {
				LogWarning << "Deflate currently unsupported, ignoring file" << header2.file_name;
				break;
			}
			default: {
				LogError << "Zip Entry: unsupported compression method " << header.compression_method;
			}
		}
		
		ifs.seekg(header.compressed_size, ifs.cur);
		
		if(header.general_purpose_bit_flag.data_descriptor_enabled) {
			zip::DataDescriptor datadescriptor;
			ifs.read((char*)&datadescriptor, sizeof(zip::DataDescriptor));
			
			if(header.crc_32 != 0 || header.compressed_size != 0 || header.uncompressed_size != 0) {
				LogWarning << "Zip Entry: invalid data";
			}
		}
	}
	
	void readCentralDirectoryHeader(fs::ifstream & ifs) {
		CentralDirectoryHeader header;
		readStruct(ifs, header);
		
		CentralDirectoryHeader2 header2;
		readString(ifs, header2.file_name, header.file_name_length);
		readString(ifs, header2.extra_field, header.extra_field_length);
		readString(ifs, header2.file_comment, header.file_comment_length);
	}
	
	void readEndOfCentralDirectoryRecord(fs::ifstream & ifs) {
		EndOfCentralDirectoryRecord header;
		readStruct(ifs, header);
		
		EndOfCentralDirectoryRecord2 header2;
		readString(ifs, header2.zip_file_comment, header.zip_file_comment_length);
		
		LogInfo << "Zip File: comment; " << header2.zip_file_comment;
	}
	
	ZipCallback & m_callback;
	
public:
	explicit ZipFileReader(ZipCallback & callback)
		: m_callback(callback)
	{ }
	
	void start(fs::ifstream & ifs) {
		ifs.seekg(0, ifs.end);
		m_fileSize = ifs.tellg();
		ifs.seekg(0);
		
		if(m_fileSize < sizeof(EndOfCentralDirectoryRecord)) {
			throw std::invalid_argument("Zip File: too small to be valid");
		}
		
		while(m_fileSize > ifs.tellg()) {
			u32 signature = peek_u32(ifs);
			
			if(signature == expected_local_file_header_signature) {
				LogInfo << "Zip File: local header at " << ifs.tellg();
				readLocalFileHeader(ifs);
			} else if(signature == expected_central_file_header_signature) {
				LogInfo << "Zip File: central header at " << ifs.tellg();
				readCentralDirectoryHeader(ifs);
			} else if(signature == expected_end_of_central_dir_signature) {
				LogInfo << "Zip File: end header at " << ifs.tellg();
				readEndOfCentralDirectoryRecord(ifs);
				size_t streamPos = ifs.tellg();
				if(m_fileSize != streamPos) {
					LogWarning << "Unexpected appended data in archive";
				}
				break;
			} else {
				throw ZipFileException("Zip File: Unexpected signature");
			}
		}
	}
};

void readFile(fs::ifstream & ifs, ZipCallback & callback) {
	zip::ZipFileReader reader(callback);
	reader.start(ifs);
}

} // namespace zip
