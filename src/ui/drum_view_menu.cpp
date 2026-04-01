#include "ui/drum_view_menu.hpp"
#include <cstring>
#include <iostream>

char DrumViewMenu::rename_input_buffer_[256];

Preset DrumViewMenu::savedCurrentPreset(std::string preset_name)
{
    int bpm = drum_controller_.getBpm();
    int drum_pack_idx = drum_controller_.getDrumPackIdx(drum_controller_.getCurrDrumPack());
    std::array<float, NUM_TRACKS> track_volumes = drum_controller_.getTrackVolumes();

    auto tracks = drum_controller_.getTracks();
    std::array<Track_t, NUM_TRACKS> track_patterns;
    for (int i = 0; i < NUM_TRACKS; i++)
    {
        track_patterns.at(i) = tracks.at(i).getTrackSequencer();
    }

    Preset new_preset(preset_name, drum_pack_idx, track_patterns, bpm, track_volumes);
    return new_preset;
}

void DrumViewMenu::onFilesDropped(int count, const char **paths)
{
    if (open_add_samples_modal_ == false)
    {
        return;
    }

    files_dropped_buf.clear();

    for (int i = 0; i < count; i++)
    {
        std::filesystem::path p = std::filesystem::path(paths[i]);

        if (std::filesystem::is_directory(p))
        {
            auto files_with_paths = FileValidation::getFilesWithRelativePaths(p);
            for (auto &file : files_with_paths)
            {
                files_dropped_buf.push_back(std::move(file));
            }
        }
        else
        {
            auto validated_files = FileValidation::validateFile(p);
            for (auto &file : validated_files)
            {
                files_dropped_buf.push_back({file, file.filename()});
            }
        }
    }
}

void DrumViewMenu::drawDrumPackSelectionMenu()
{
    std::string curr_drum_pack = drum_controller_.getCurrDrumPack();
    std::vector<std::string> drum_packs = drum_controller_.getDrumPacks();

    static int selected = 0;
    int controller_idx = drum_controller_.getDrumPackIdx(curr_drum_pack);
    if (controller_idx >= 0 && controller_idx < static_cast<int>(drum_packs.size()))
    {
        selected = controller_idx;
    }

    for (size_t i = 0; i < drum_packs.size(); i++)
    {
        std::string name = drum_packs.at(i);
        if (ImGui::Selectable(name.c_str(), selected == static_cast<int>(i)))
        {
            selected = static_cast<int>(i);
            drum_controller_.setDrumPack(static_cast<int>(i));
        }
    }
}

void DrumViewMenu::drawDrumPackCreationMenu()
{
    if (ImGui::Selectable("Create New Drum Pack"))
    {
        sample_structure_ = FileUtils::getSamplesDirectoryStructure("assets/samples");
        root_sample_selections_.clear();
        folder_selections_.clear();
        for (const auto &sample : sample_structure_.root_samples)
        {
            root_sample_selections_[sample.filename().string()] = false;
        }
        open_create_drum_pack_modal_ = true;
    }
}

void DrumViewMenu::drawDeleteDrumPackSubMenu()
{
    auto drum_packs = drum_controller_.getDrumPacks();

    if (delete_drum_pack_selected_index_ >= static_cast<int>(drum_packs.size()))
    {
        delete_drum_pack_selected_index_ = 0;
    }

    for (size_t i = 0; i < drum_packs.size(); i++)
    {
        std::string name = drum_packs.at(i);
        if (ImGui::Selectable(name.c_str(), delete_drum_pack_selected_index_ == static_cast<int>(i),
                              ImGuiSelectableFlags_DontClosePopups))
        {
            delete_drum_pack_selected_index_ = static_cast<int>(i);
        }
    }

    ImGui::Separator();

    if (drum_packs.empty())
    {
        ImGui::TextDisabled("No drum packs to delete");
    }
    else
    {
        if (ImGui::Button("Delete Selected Drum Pack"))
        {
            ImGui::OpenPopup("ConfirmDeleteDrumPack");
        }
    }

    // Confirmation Modal
    if (ImGui::BeginPopupModal("ConfirmDeleteDrumPack", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete \"%s\"?", drum_packs.at(delete_drum_pack_selected_index_).c_str());
        ImGui::Spacing();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
            drum_controller_.deleteDrumPack(delete_drum_pack_selected_index_);
            delete_drum_pack_selected_index_ = 0;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawFileMenu()
{
    Preset main_session = savedCurrentPreset("Main Session");
    if (ImGui::MenuItem("Save Session", NULL, false, true))
    {
        drum_controller_.saveSession(main_session);
    }

    if (ImGui::MenuItem("Add Samples", NULL, false, true))
    {
        open_add_samples_modal_ = true;
    }

    if (ImGui::MenuItem("Rearrange Tracks", NULL, false, true))
    {
        open_rearrange_tracks_modal_ = true;
    }
}

void DrumViewMenu::drawPresetsMenu()
{
    auto presets_list = drum_controller_.getPresetsList();

    static int selected = 0;
    for (size_t i = 0; i < presets_list.size(); i++)
    {
        std::string preset_name = presets_list.at(i).getPresetName();
        if (ImGui::Selectable(preset_name.c_str(), selected == i))
        {
            selected = i;
            drum_controller_.loadPreset(presets_list.at(i));
        }
    }
}

void DrumViewMenu::drawDeleteSubMenu()
{
    auto presets_list = drum_controller_.getPresetsList();

    static int selected = 0;
    for (size_t i = 0; i < presets_list.size(); i++)
    {
        std::string preset_name = presets_list.at(i).getPresetName();
        if (ImGui::Selectable(preset_name.c_str(), selected == i, ImGuiSelectableFlags_DontClosePopups))
        {
            selected = i;
        }
    }

    if (ImGui::Button("Delete Selected Preset"))
    {
        drum_controller_.deletePreset(selected);
        ImGui::CloseCurrentPopup();
    }
}

void DrumViewMenu::drawSavePresetModal()
{

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("SavePreset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {

        static char preset_name[64];
        ImGui::Spacing();
        ImGui::InputTextWithHint("##PresetName", "Preset Name", preset_name, sizeof(preset_name));
        ImGui::Spacing();

        Preset new_preset = savedCurrentPreset(preset_name);

        ImGui::Spacing();

        if (preset_name[0] == '\0')
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Save Preset", ImVec2(120, 0)))
        {
            drum_controller_.addPreset(new_preset);
            open_save_preset_modal_ = false;
            preset_name[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        if (preset_name[0] == '\0')
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            open_save_preset_modal_ = false;
            preset_name[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawAddSamplesModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AddSamples", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Drop Your Samples Here");

        ImGui::SameLine();
        ImGui::TextDisabled("(?)");

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Double click the file to remove");
        }

        ImVec2 list_box_size = ImVec2(0, 0);
        static float max_text_width = 0.0f;

        for (auto &file : files_dropped_buf)
        {
            if (std::filesystem::exists(file.first))
            {
                float w = ImGui::CalcTextSize(file.first.string().c_str()).x;

                max_text_width = (std::max)(max_text_width, w);
                files_accepted_.insert(std::move(file));
            }
        }

        files_dropped_buf.clear();

        if (files_accepted_.size() > 0)
        {
            float padding_x = 15.0f;
            list_box_size.x = max_text_width + padding_x;
        }

        if (ImGui::BeginListBox("##Dropped Samples", list_box_size))
        {
            std::set<path_pair_t>::iterator to_erase = files_accepted_.end();

            for (auto it = files_accepted_.begin(); it != files_accepted_.end(); ++it)
            {
                const auto &file = *it;

                if (ImGui::Selectable(file.first.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        to_erase = it;
                    }
                }
            }

            if (to_erase != files_accepted_.end())
            {
                files_accepted_.erase(to_erase);
            }

            ImGui::EndListBox();
        }

        ImGui::BeginDisabled(files_accepted_.empty());
        if (ImGui::Button("Submit"))
        {
            drum_controller_.startCopyQueue(files_accepted_);
            files_accepted_.clear();
            open_add_samples_modal_ = false;
            open_copy_progress_modal_ = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            files_accepted_.clear();
            files_dropped_buf.clear();
            ImGui::CloseCurrentPopup();
            open_add_samples_modal_ = false;
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawCopyConflictModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyConflict", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::string conflict_name = drum_controller_.getCurrentConflictFilename();
        ImGui::Text("File \"%s\" already exists.", conflict_name.c_str());
        ImGui::Spacing();

        ImGui::Text("Rename to:");
        ImGui::SameLine();

        std::string current_name = drum_controller_.getCurrentConflictFilename();
        std::string stem;
        if (!current_name.empty())
        {
            std::filesystem::path p(current_name);
            stem = p.stem().string();
        }

        if (rename_input_buffer_[0] == '\0')
        {
            strncpy(rename_input_buffer_, stem.c_str(), sizeof(rename_input_buffer_) - 1);
            rename_input_buffer_[sizeof(rename_input_buffer_) - 1] = '\0';
        }

        ImGui::InputText("##RenameInput", rename_input_buffer_, sizeof(rename_input_buffer_));

        ImGui::Spacing();

        if (ImGui::Button("Skip"))
        {
            drum_controller_.skipCurrentFile();
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool can_rename = strlen(rename_input_buffer_) > 0;

        ImGui::BeginDisabled(!can_rename);
        if (ImGui::Button("Rename"))
        {
            drum_controller_.renameAndCopyCurrentFile(std::string(rename_input_buffer_));
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Replace"))
        {
            drum_controller_.replaceCurrentFile();
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Skip All"))
        {
            drum_controller_.finishCopy();
            memset(rename_input_buffer_, 0, sizeof(rename_input_buffer_));

            open_copy_progress_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawCopyProgressModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float y_offset = 100.0f;

    ImGui::SetNextWindowSize(ImVec2(avail.x / 2, avail.y / 4));
    ImGui::SetNextWindowPos(ImVec2(center.x, center.y + y_offset), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyProgress", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool has_conflict = drum_controller_.hasCopyConflict();
        std::string copying_file = drum_controller_.getCurrentCopyingFilename();

        int remaining = drum_controller_.getCopyQueueRemaining();
        ImGui::Text("Copying: %s", copying_file.c_str());
        ImGui::Spacing();

        ImGui::Text("%d files remaining", remaining);
        ImGui::Spacing();

        if (ImGui::Button("Cancel"))
        {
            drum_controller_.finishCopy();
            open_copy_progress_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        if (has_conflict)
        {
            ImGui::OpenPopup("CopyConflict");
            drawCopyConflictModal();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawCopyCompletionModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("CopyCompletion", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto successful = drum_controller_.getSuccessfulCopies();
        auto errors = drum_controller_.getCopyErrors();

        ImGui::Text("Copy Complete!");
        ImGui::Spacing();

        ImGui::Text("Successfully copied: %zu file(s)", successful.size());

        if (!errors.empty())
        {
            ImGui::Text("Errors: %zu", errors.size());
            for (const auto &[file, msg] : errors)
            {
                ImGui::BulletText("%s: %s", file.c_str(), msg.c_str());
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Done"))
        {
            open_copy_completion_modal_ = false;
            open_copy_progress_modal_ = false;
            open_add_samples_modal_ = false;

            drum_controller_.finishCopy();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawCreateDrumPackModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("CreateDrumPack", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Drum Pack Name:");
        static char pack_name_buffer[64] = "New Drum Pack";
        ImGui::InputText("##PackName", pack_name_buffer, sizeof(pack_name_buffer));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Select Samples:");
        ImGui::Spacing();

        int selected_count = 0;
        for (const auto &pair : root_sample_selections_)
        {
            if (pair.second)
            {
                selected_count++;
            }
        }
        for (const auto &fs : folder_selections_)
        {
            for (const auto &pair : fs.sample_selections)
            {
                if (pair.second)
                {
                    selected_count++;
                }
            }
        }

        const float child_height = 280.0f;

        ImGui::BeginChild("SampleList", ImVec2(0, child_height), true);

        if (!sample_structure_.root_samples.empty())
        {
            if (ImGui::CollapsingHeader("Root Samples"))
            {
                ImGui::Indent();
                for (const auto &sample : sample_structure_.root_samples)
                {
                    std::string sample_name = sample.filename().string();
                    bool selected = root_sample_selections_[sample_name];

                    ImGui::BeginDisabled(selected_count >= NUM_TRACKS && !selected);
                    if (ImGui::Checkbox(sample_name.c_str(), &selected))
                    {
                        root_sample_selections_[sample_name] = selected;
                    }
                    ImGui::EndDisabled();
                }
                ImGui::Unindent();
            }
        }

        for (const auto &folder : sample_structure_.folders)
        {
            if (ImGui::CollapsingHeader(folder.name.c_str()))
            {
                ImGui::Indent();
                for (const auto &sample : folder.samples)
                {
                    std::string sample_name = sample.filename().string();
                    bool selected = false;

                    for (auto &fs : folder_selections_)
                    {
                        if (fs.folder_name == folder.name)
                        {
                            auto it = fs.sample_selections.find(sample_name);
                            if (it != fs.sample_selections.end())
                            {
                                selected = it->second;
                            }
                            break;
                        }
                    }

                    std::string id = folder.name + "_" + sample_name;
                    ImGui::PushID(id.c_str());

                    ImGui::BeginDisabled(selected_count >= NUM_TRACKS && !selected);
                    if (ImGui::Checkbox(sample_name.c_str(), &selected))
                    {
                        bool found = false;
                        for (auto &fs : folder_selections_)
                        {
                            if (fs.folder_name == folder.name)
                            {
                                fs.sample_selections[sample_name] = selected;
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            SampleSelection fs;
                            fs.folder_name = folder.name;
                            fs.sample_selections[sample_name] = selected;
                            folder_selections_.push_back(fs);
                        }
                    }
                    ImGui::EndDisabled();
                    ImGui::PopID();
                }
                ImGui::Unindent();
            }
        }

        ImGui::EndChild();

        ImGui::Spacing();

        std::array<std::filesystem::path, NUM_TRACKS> selected_samples;
        int sample_idx = 0;

        std::filesystem::path samples_root = drum_controller_.getSamplesRootDir();

        for (const auto &pair : root_sample_selections_)
        {
            if (pair.second && sample_idx < NUM_TRACKS)
            {
                selected_samples.at(sample_idx) = samples_root / pair.first;
                sample_idx++;
            }
        }

        for (const auto &fs : folder_selections_)
        {
            for (const auto &pair : fs.sample_selections)
            {
                if (pair.second && sample_idx < NUM_TRACKS)
                {
                    selected_samples.at(sample_idx) = samples_root / fs.folder_name / pair.first;
                    sample_idx++;
                }
            }
        }

        // Fill remaining with empty paths if less than NUM_TRACKS selected
        for (int i = sample_idx; i < NUM_TRACKS; i++)
        {
            selected_samples.at(i) = std::filesystem::path();
        }

        ImGui::Text("%d/%d sample(s) selected", selected_count, NUM_TRACKS);
        ImGui::Spacing();

        bool can_create = selected_count > 0 && strlen(pack_name_buffer) > 0;

        if (ImGui::Button("Create", ImVec2(120, 0)) && can_create)
        {
            drum_controller_.addDrumPack(pack_name_buffer, selected_samples);
            root_sample_selections_.clear();
            folder_selections_.clear();
            open_create_drum_pack_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            open_create_drum_pack_modal_ = false;
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawRearrangeTracksModal()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Rearrange Tracks", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Drag tracks to reorder");

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.137f, 0.216f, 0.333f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 6.0f));

        ImGui::BeginListBox("##RearrangeTracks", ImVec2(0, 300));
        auto tracks = drum_controller_.getTracks();

        for (int i = 0; i < tracks.size(); i++)
        {
            ImGui::PushID(i);
            std::string track_name = tracks.at(i).getName();

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.102f, 0.165f, 0.278f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.137f, 0.216f, 0.333f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.071f, 0.102f, 0.176f, 1.0f));

            ImGui::Selectable(track_name.c_str(), false);

            ImGui::PopStyleColor(3);

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload("TRACK_INDEX", &i, sizeof(int));
                ImGui::Text("%s", track_name.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TRACK_INDEX"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(int));
                    int src_idx = *(const int *)payload->Data;
                    int dst_idx = i;

                    if (src_idx != dst_idx)
                    {
                        drum_controller_.swapTracks(src_idx, dst_idx);
                        // refresh local copy of tracks after swap
                        tracks = drum_controller_.getTracks();
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }

        ImGui::EndListBox();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        if (ImGui::Button("Done"))
        {
            open_rearrange_tracks_modal_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrumViewMenu::drawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            drawFileMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Drum Packs"))
        {
            drawDrumPackSelectionMenu();

            ImGui::MenuItem("Create New Kit", NULL, false, false);
            drawDrumPackCreationMenu();

            ImGui::Separator();

            if (ImGui::BeginMenu("Remove Drum Packs"))
            {
                drawDeleteDrumPackSubMenu();
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Presets"))
        {
            drawPresetsMenu();
            ImGui::NewLine();
            ImGui::Separator();

            if (ImGui::MenuItem("Save Current as Preset", NULL, false, true))
            {
                open_save_preset_modal_ = true;
            }

            if (ImGui::BeginMenu("Remove Presets"))
            {
                drawDeleteSubMenu();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (open_save_preset_modal_)
    {
        ImGui::OpenPopup("SavePreset");
    }

    if (open_add_samples_modal_)
    {
        ImGui::OpenPopup("AddSamples");
    }

    if (open_copy_progress_modal_)
    {
        ImGui::OpenPopup("CopyProgress");
    }

    if (open_copy_completion_modal_)
    {
        ImGui::OpenPopup("CopyCompletion");
    }

    if (open_create_drum_pack_modal_)
    {
        ImGui::OpenPopup("CreateDrumPack");
    }

    if (open_rearrange_tracks_modal_)
    {
        ImGui::OpenPopup("Rearrange Tracks");
    }

    drawSavePresetModal();
    drawAddSamplesModal();
    drawRearrangeTracksModal();
    drawCopyProgressModal();
    drawCopyConflictModal();
    drawCopyCompletionModal();
    drawCreateDrumPackModal();

    if (drum_controller_.isCopyingInProgress())
    {
        drum_controller_.processNextCopy();
    }

    if (drum_controller_.getCopyQueueRemaining() == 0 && !drum_controller_.hasCopyConflict())
    {
        if (open_copy_progress_modal_)
        {
            drum_controller_.finishCopy();
            open_copy_progress_modal_ = false;
            open_copy_completion_modal_ = true;
        }
    }
}
