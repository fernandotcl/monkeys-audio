#include "All.h"
#include "WAVInputSource.h"
#include IO_HEADER_FILE
#include "MACLib.h"
#include "GlobalFunctions.h"

struct RIFF_HEADER 
{
    char cRIFF[4];            // the characters 'RIFF' indicating that it's a RIFF file
    uint32 nBytes;    // the number of bytes following this header
};

struct DATA_TYPE_ID_HEADER 
{
    char cDataTypeID[4];    // should equal 'WAVE' for a WAV file
};

struct WAV_FORMAT_HEADER
{
    uint16 nFormatTag;            // the format of the WAV...should equal 1 for a PCM file
    uint16 nChannels;            // the number of channels
    uint32 nSamplesPerSecond;    // the number of samples per second
    uint32 nBytesPerSecond;        // the bytes per second
    uint16 nBlockAlign;            // block alignment
    uint16 nBitsPerSample;        // the number of bits per sample
};

struct RIFF_CHUNK_HEADER
{
    char cChunkLabel[4];        // should equal "data" indicating the data chunk
    uint32 nChunkBytes;  // the bytes of the chunk  
};

unsigned long LE_LONG(unsigned char * buf)
/* converts 4 bytes stored in little-endian format to an unsigned long */
{
	return (unsigned long)((buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0]);
}

unsigned short LE_SHORT(unsigned char * buf)
/* converts 2 bytes stored in little-endian format to an unsigned short */
{
	return (unsigned short)((buf[1] << 8) + buf[0]);
}

CInputSource * CreateInputSource(const wchar_t * pSourceName, WAVEFORMATEX * pwfeSource, int * pTotalBlocks, int * pHeaderBytes, int * pTerminatingBytes, int * pErrorCode)
{ 
	return new CWAVInputSource(pSourceName, pwfeSource, pTotalBlocks, pHeaderBytes, pTerminatingBytes, pErrorCode);
}

CWAVInputSource::CWAVInputSource(CIO * pIO, WAVEFORMATEX * pwfeSource, int * pTotalBlocks, int * pHeaderBytes, int * pTerminatingBytes, int * pErrorCode)
    : CInputSource(pIO, pwfeSource, pTotalBlocks, pHeaderBytes, pTerminatingBytes, pErrorCode)
{
    m_bIsValid = FALSE;

    if (pIO == NULL || pwfeSource == NULL)
    {
        if (pErrorCode) *pErrorCode = ERROR_BAD_PARAMETER;
        return;
    }
    
    m_spIO.Assign(pIO, FALSE, FALSE);

    int nRetVal = AnalyzeSource();
    if (nRetVal == ERROR_SUCCESS)
    {
        // fill in the parameters
        if (pwfeSource) memcpy(pwfeSource, &m_wfeSource, sizeof(WAVEFORMATEX));
        if (pTotalBlocks) *pTotalBlocks = m_nDataBytes / m_wfeSource.nBlockAlign;
        if (pHeaderBytes) *pHeaderBytes = m_nHeaderBytes;
        if (pTerminatingBytes) *pTerminatingBytes = m_nTerminatingBytes;

        m_bIsValid = TRUE;
    }
    
    if (pErrorCode) *pErrorCode = nRetVal;
}

CWAVInputSource::CWAVInputSource(const wchar_t * pSourceName, WAVEFORMATEX * pwfeSource, int * pTotalBlocks, int * pHeaderBytes, int * pTerminatingBytes, int * pErrorCode)
    : CInputSource(pSourceName, pwfeSource, pTotalBlocks, pHeaderBytes, pTerminatingBytes, pErrorCode)
{
    m_bIsValid = FALSE;

    if (pSourceName == NULL || pwfeSource == NULL)
    {
        if (pErrorCode) *pErrorCode = ERROR_BAD_PARAMETER;
        return;
    }
    
    m_spIO.Assign(new IO_CLASS_NAME);
    if (m_spIO->Open(pSourceName) != ERROR_SUCCESS)
    {
        m_spIO.Delete();
        if (pErrorCode) *pErrorCode = ERROR_INVALID_INPUT_FILE;
        return;
    }

    int nRetVal = AnalyzeSource();
    if (nRetVal == ERROR_SUCCESS)
    {
        // fill in the parameters
        if (pwfeSource) memcpy(pwfeSource, &m_wfeSource, sizeof(WAVEFORMATEX));
        if (pTotalBlocks) *pTotalBlocks = m_nDataBytes / m_wfeSource.nBlockAlign;
        if (pHeaderBytes) *pHeaderBytes = m_nHeaderBytes;
        if (pTerminatingBytes) *pTerminatingBytes = m_nTerminatingBytes;

        m_bIsValid = TRUE;
    }
    
    if (pErrorCode) *pErrorCode = nRetVal;
}

CWAVInputSource::~CWAVInputSource()
{


}

int CWAVInputSource::AnalyzeSource()
{
	unsigned char *p = FULL_HEADER, *priff = NULL;

	// seek to the beginning (just in case)
	m_spIO->Seek(0, FILE_BEGIN);

	// get the file size
	m_nFileBytes = m_spIO->GetSize();

	// get the RIFF header
	RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(RIFF_HEADER)))

	// make sure the RIFF header is valid
	if (!(p[0] == 'R' && p[1] == 'I' && p[2] == 'F' && p[3] == 'F'))
		return ERROR_INVALID_INPUT_FILE;
	p += sizeof(RIFF_HEADER);

	// read the data type header
	RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(DATA_TYPE_ID_HEADER)))

	// make sure it's the right data type
	if (!(p[0] == 'W' && p[1] == 'A' && p[2] == 'V' && p[3] == 'E'))
		return ERROR_INVALID_INPUT_FILE;
	p += sizeof(DATA_TYPE_ID_HEADER);

	// find the 'fmt ' chunk
	RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(RIFF_CHUNK_HEADER)))

	while (!(p[0] == 'f' && p[1] == 'm' && p[2] == 't' && p[3] == ' '))
	{
		p += sizeof(RIFF_CHUNK_HEADER);

		// move the file pointer to the end of this chunk
		RETURN_ON_ERROR(ReadSafe(m_spIO, p, LE_LONG(p+4)))
		p += LE_LONG(p+4);

		// check again for the data chunk
		RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(RIFF_CHUNK_HEADER)))
    }

	priff = p+4;
	p += sizeof(RIFF_CHUNK_HEADER);

	// read the format info
	RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(WAV_FORMAT_HEADER)))

	// error check the header to see if we support it
	if (LE_SHORT(p) != 1)
		return ERROR_INVALID_INPUT_FILE;

	// copy the format information to the WAVEFORMATEX passed in
	FillWaveFormatEx(&m_wfeSource, LE_LONG(p+4), LE_SHORT(p+14), LE_SHORT(p+2));

	p += sizeof(WAV_FORMAT_HEADER);

	// skip over any extra data in the header
	int nWAVFormatHeaderExtra = LE_LONG(priff) - sizeof(WAV_FORMAT_HEADER);
	if (nWAVFormatHeaderExtra < 0)
		return ERROR_INVALID_INPUT_FILE;
	else {
		RETURN_ON_ERROR(ReadSafe(m_spIO, p, nWAVFormatHeaderExtra))
		p += nWAVFormatHeaderExtra;
	}

	// find the data chunk
	RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(RIFF_CHUNK_HEADER)))

	while (!(p[0] == 'd' && p[1] == 'a' && p[2] == 't' && p[3] == 'a'))
	{
		p += sizeof(RIFF_CHUNK_HEADER);

		// move the file pointer to the end of this chunk
		RETURN_ON_ERROR(ReadSafe(m_spIO, p, LE_LONG(p+4)))
		p += LE_LONG(p+4);

		// check again for the data chunk
		RETURN_ON_ERROR(ReadSafe(m_spIO, p, sizeof(RIFF_CHUNK_HEADER)))
	}

	// we're at the data block
	m_nDataBytes = LE_LONG(p+4);
	if (m_nDataBytes < 0)
		m_nDataBytes = m_nFileBytes - m_nHeaderBytes;

	p += sizeof(RIFF_CHUNK_HEADER);

	m_nHeaderBytes = p - FULL_HEADER;

	// make sure the data bytes is a whole number of blocks
	if ((m_nDataBytes % m_wfeSource.nBlockAlign) != 0)
		return ERROR_INVALID_INPUT_FILE;

	// calculate the terminating byts
	m_nTerminatingBytes = 0;

	// we made it this far, everything must be cool
	return ERROR_SUCCESS;
}

int CWAVInputSource::GetData(unsigned char * pBuffer, int nBlocks, int * pBlocksRetrieved)
{
    if (!m_bIsValid) return ERROR_UNDEFINED;

    int nBytes = (m_wfeSource.nBlockAlign * nBlocks);
    unsigned int nBytesRead = 0;

    if (m_spIO->Read(pBuffer, nBytes, &nBytesRead) != ERROR_SUCCESS)
        return ERROR_IO_READ;

    if (pBlocksRetrieved) *pBlocksRetrieved = (nBytesRead / m_wfeSource.nBlockAlign);

    return ERROR_SUCCESS;
}

int CWAVInputSource::GetHeaderData(unsigned char * pBuffer)
{
	int i;

	if (!m_bIsValid) return ERROR_UNDEFINED;

	for (i=0;i<m_nHeaderBytes;i++)
		*(pBuffer + i) = *(FULL_HEADER + i);

	return ERROR_SUCCESS;
}

int CWAVInputSource::GetTerminatingData(unsigned char * pBuffer)
{
    if (!m_bIsValid) return ERROR_UNDEFINED;

    int nRetVal = ERROR_SUCCESS;

    if (m_nTerminatingBytes > 0)
    {
        int nOriginalFileLocation = m_spIO->GetPosition();

        m_spIO->Seek(-m_nTerminatingBytes, FILE_END);
        
        unsigned int nBytesRead = 0;
        int nReadRetVal = m_spIO->Read(pBuffer, m_nTerminatingBytes, &nBytesRead);

        if ((nReadRetVal != ERROR_SUCCESS) || (m_nTerminatingBytes != int(nBytesRead)))
        {
            nRetVal = ERROR_UNDEFINED;
        }

        m_spIO->Seek(nOriginalFileLocation, FILE_BEGIN);
    }

    return nRetVal;
}
