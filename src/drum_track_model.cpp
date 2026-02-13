#include "drum_track_model.hpp"

DrumTrackModel::DrumTrackModel(const std::string &name, const std::string &samplePath) : samplePath_(samplePath), name_(name)
{
    sequencer_.fill(false);
}

DrumTrackModel::DrumTrackModel() : samplePath_(), name_()
{
    sequencer_.fill(false);
}

DrumTrackModel::~DrumTrackModel() = default;

std::string DrumTrackModel::getName()
{
    return name_;
}

std::string DrumTrackModel::getSample()
{
    return samplePath_;
}

Track_t &DrumTrackModel::getTrackSequencer()
{
    return sequencer_;
}

void DrumTrackModel::setName(std::string &newName)
{
    name_ = newName;
}

void DrumTrackModel::setSample(std::string &samplePath)
{
    samplePath_ = samplePath;
}
