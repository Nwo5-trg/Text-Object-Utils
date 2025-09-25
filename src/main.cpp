#include <Geode/Geode.hpp>
#include <Geode/modify/CustomizeObjectLayer.hpp>
#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

class $modify(EditTextLayer, CustomizeObjectLayer) {
    struct Fields {
        std::vector<TextGameObject*> textObjects;

        TextInput* kerningInput = nullptr;
        CCMenu* textObjectUtilsMenu;

        bool swapCopyPaste = Mod::get()->getSettingValue<bool>("swap-copy-paste-buttons");
        std::string lineShortcut = Mod::get()->getSettingValue<std::string>("new-line-shortcut");
    };

    bool init(GameObject* p0, CCArray* p1) {
        if (!CustomizeObjectLayer::init(p0, p1)) return false;
    
        if (!m_textInput) return true;

        auto fields = m_fields.self();

        if (fields->lineShortcut.empty()) fields->lineShortcut = "\\n";

        for (auto obj : CCArrayExt<GameObject>(EditorUI::get()->getSelectedObjects())) {
            if (auto text = typeinfo_cast<TextGameObject*>(obj)) fields->textObjects.push_back(text);
        }

        m_textInput->setPositionY(m_textInput->getPositionY() - 20);

        auto inputBG = static_cast<CCScale9Sprite*>(m_mainLayer->getChildByID("text-input-bg"));
        inputBG->setPositionY(inputBG->getPositionY() - 20);
        inputBG->setContentSize({inputBG->getContentWidth() - 40, inputBG->getContentHeight()});

        m_kerningSlider->setPositionY(m_kerningSlider->getPositionY() - 20);
        m_kerningLabel->setPosition(m_kerningLabel->getPosition() - ccp(26.0f, 20.0f));

        this->getChildByIDRecursive("clear-text-button")->setPositionY(500.0f); // why not keep it? idk :3c

        // character bypass
        m_textInput->setMaxLabelLength(99999);
        
        // open text edit
        if (Loader::get()->isModLoaded("hjfod.betteredit")) {
            this->scheduleOnce(schedule_selector(EditTextLayer::openTextMenu), 0);
        }
        else {
            openTextMenu(0.0f);
        }
 
        // make my menus and stuff
        fields->textObjectUtilsMenu = CCMenu::create();
        fields->textObjectUtilsMenu->setPosition(0.0f, 0.0f);
        fields->textObjectUtilsMenu->setID("text-object-utils-menu"_spr);
        m_mainLayer->addChild(fields->textObjectUtilsMenu);
        m_textTabNodes->addObject(fields->textObjectUtilsMenu);

        createButton( // copy button
            "GJ_copyBtn_001.png", menu_selector(EditTextLayer::onCopyText), "copy-text-button"_spr,
            {inputBG->getPositionX() + (fields->swapCopyPaste ? -120 : 120), inputBG->getPositionY()}, 0.5f
        );

        createButton( // paste button
            "GJ_pasteBtn_001.png", menu_selector(EditTextLayer::onPasteText), "paste-text-button"_spr,
            {inputBG->getPositionX() + (fields->swapCopyPaste ? 120 : -120), inputBG->getPositionY()}, 0.5f
        );

        createButton( // clear button
            "GJ_trashBtn_001.png", menu_selector(EditTextLayer::onClearText), "clear-text-button"_spr,
            {inputBG->getPositionX() + 150, inputBG->getPositionY()}, 0.55f
        );

        createButton( // newline button
            "GJ_redoBtn_001.png", menu_selector(EditTextLayer::onNewlineText), "newline-text-button"_spr,
            {inputBG->getPositionX() - 150, inputBG->getPositionY()}, 0.6f
        );

        auto kerningInput = TextInput::create(60.0f, "");
        kerningInput->setID("kerning-input"_spr);
        kerningInput->setCommonFilter(CommonFilter::Int);
        kerningInput->setCallback([this] (const std::string& str) {
            if (str.empty()) return;

            int kerning = numFromString<int>(str).unwrapOr(0);
            m_kerningAmount = kerning;
            m_kerningSlider->setValue(std::clamp(kerning + 10.0f, 0.0f, 30.0f) / 30);

            for (auto obj : m_fields->textObjects) {
                obj->updateTextKerning(kerning);
            }

            updateKerningLabel();
        });
        kerningInput->setScale(0.7f);
        m_mainLayer->addChild(kerningInput);
        m_textTabNodes->addObject(kerningInput);
        fields->kerningInput = kerningInput;
        updateKerningLabel();
        
        return true;
    }

    void createButton(const char* sprite, SEL_MenuHandler selector, std::string id, CCPoint pos, float scale) {
        auto clearButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName(sprite), this, selector
        );
        clearButton->setID(id);
        clearButton->setPosition(pos);
        clearButton->setScale(scale);
        clearButton->m_baseScale = scale;
        m_fields->textObjectUtilsMenu->addChild(clearButton);
    }

    void textChanged(CCTextInputNode* p0) {
        auto fields = m_fields.self();

        if (p0 != m_textInput) return;

        std::string str = p0->getString();
        
        if (str.ends_with(fields->lineShortcut)) {
            str.erase(str.length() - fields->lineShortcut.length());
            str.push_back('\n');
            p0->setString(str);
        }
        else {
            CustomizeObjectLayer::textChanged(p0);
        }
    }

    void updateKerningLabel() {
        CustomizeObjectLayer::updateKerningLabel();

        auto fields = m_fields.self();

        if (!fields->kerningInput || !m_kerningLabel) return;

        std::string str = m_kerningLabel->getString();
        auto end = str.find(' ');
        if (end != std::string::npos) str.erase(end);
        m_kerningLabel->setString(str.c_str());


        fields->kerningInput->setPosition(
            m_kerningLabel->getPosition() + ccp((m_kerningLabel->getContentWidth() / 2) + 10, 0.0f)
        );
        fields->kerningInput->setString(numToString(m_kerningAmount));
    }

    void onCopyText(CCObject* sender) {
        if (m_textInput) clipboard::write(m_textInput->getString());
    }
    void onPasteText(CCObject* sender) {
        if (m_textInput) m_textInput->setString(clipboard::read());
    }
    void onClearText(CCObject* sender) {
        if (m_textInput) m_textInput->setString("");
    }
    void onNewlineText(CCObject* sender) {
        if (!m_textInput) return;
        auto str = m_textInput->getString();
        str.push_back('\n');
        m_textInput->setString(str);
    }

    void openTextMenu(float dt) {
        if (auto button = m_textButton) button->activate();
    } 
    void onClose(CCObject* sender) {
        // god just add this to the destructor or sum shi its annoying
        if (m_fields->kerningInput) m_fields->kerningInput->getInputNode()->onClickTrackNode(false);
        CustomizeObjectLayer::onClose(sender);
    }
};