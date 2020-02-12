#include <flatbuffers/flatbuffers.h>
#include <fstream>
#include <pl72_run_start_generated.h>
#include <sstream>

#include "DetectorSpectrumMapData.h"

bool file_exists(const std::string &name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  }
  return false;
}

DetectorSpectrumMapData::DetectorSpectrumMapData(const std::string &filename) {
  if (filename.empty()) {
    throw std::runtime_error("A detector-spectrum map file must be specified.");
  }
  if (!file_exists(filename))
    throw std::runtime_error(
        filename +
        " cannot be found, please check full path is given and correct.");
  readFile(filename);
}

DetectorSpectrumMapData::DetectorSpectrumMapData(
    const SpectraDetectorMapping *detSpecMapFromMessage) {
  const auto detFBVector = detSpecMapFromMessage->detector_id();
  const auto specFBVector = detSpecMapFromMessage->spectrum();
  setNumberOfEntries(detSpecMapFromMessage->n_spectra());
  m_detectors.resize(static_cast<size_t>(m_numberOfEntries));
  m_spectra.resize(static_cast<size_t>(m_numberOfEntries));
  std::copy(detFBVector->cbegin(), detFBVector->cend(), m_detectors.begin());
  std::copy(specFBVector->cbegin(), specFBVector->cend(), m_spectra.begin());
}

void DetectorSpectrumMapData::readFile(const std::string &filename) {
  std::ifstream infile(filename);
  std::string line;
  std::getline(infile, line); // discard first line
  std::getline(infile, line); // line with the number of entries
  std::istringstream iss(line);
  iss >> m_numberOfEntries;
  m_detectors.resize(static_cast<size_t>(m_numberOfEntries));
  m_spectra.resize(static_cast<size_t>(m_numberOfEntries));
  std::getline(infile, line); // discard third line
  size_t entryNumber = 0;
  while (std::getline(infile, line)) {
    std::istringstream isstream(line);
    if (!(isstream >> m_detectors[entryNumber] >> m_spectra[entryNumber])) {
      break;
    } // error

    entryNumber++;
  }
}

flatbuffers::Offset<SpectraDetectorMapping>
DetectorSpectrumMapData::addToBuffer(flatbuffers::FlatBufferBuilder &builder) {
  return CreateSpectraDetectorMapping(builder, builder.CreateVector(m_spectra),
                                      builder.CreateVector(m_detectors),
                                      m_numberOfEntries);
}
