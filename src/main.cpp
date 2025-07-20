#include <Geode/Geode.hpp>
#include <Geode/modify/CustomizeObjectLayer.hpp>
#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

class $modify(EditTextLayer, CustomizeObjectLayer) {
    struct Fields {
        TextGameObject* textObject = nullptr;
        bool swapCopyPaste = Mod::get()->getSettingValue<bool>("swap-copy-paste-buttons");
        std::string lineShortcut = Mod::get()->getSettingValue<std::string>("new-line-shortcut");
    };

    bool init(GameObject* p0, CCArray* p1) {
        if (!CustomizeObjectLayer::init(p0, p1)) return false;
    
        if (!m_textInput) return true;

        auto& lineShortcut = m_fields->lineShortcut;
        if (lineShortcut.empty()) lineShortcut = "/n"; // fuck whatever user trys this


        m_fields->textObject = static_cast<TextGameObject*>(p0);
        m_textInput->setPositionY(m_textInput->getPositionY() - 20);
        auto inputBG = static_cast<CCScale9Sprite*>(m_mainLayer->getChildByID("text-input-bg"));
        inputBG->setPositionY(inputBG->getPositionY() - 20);
        inputBG->setContentSize({inputBG->getContentWidth() - 40, inputBG->getContentHeight()});
        m_kerningLabel->setPositionY(m_kerningLabel->getPositionY() - 20);
        m_kerningSlider->setPositionY(m_kerningSlider->getPositionY() - 20);
        this->getChildByIDRecursive("clear-text-button")->setPositionY(500.0f); // why not keep it? idk :3c
        
        if (Loader::get()->isModLoaded("hjfod.betteredit")) this->scheduleOnce(schedule_selector(EditTextLayer::openTextMenu), 0);
        else openTextMenu(0.0f);
 
        auto menu = CCMenu::create();
        menu->setPosition(0.0f, 0.0f);
        menu->setID("text-object-utils-menu"_spr);
        m_mainLayer->addChild(menu);
        m_textTabNodes->addObject(menu);

        auto copyButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_copyBtn_001.png"), this, menu_selector(EditTextLayer::onCopyText));
        copyButton->setPosition(inputBG->getPositionX() + (m_fields->swapCopyPaste ? -120 : 120), inputBG->getPositionY());
        copyButton->setScale(0.5f);
        copyButton->m_baseScale = 0.5f;
        menu->addChild(copyButton);

        auto pasteButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_pasteBtn_001.png"), this, menu_selector(EditTextLayer::onPasteText));
        pasteButton->setPosition(inputBG->getPositionX() + (m_fields->swapCopyPaste ? 120 : -120), inputBG->getPositionY());
        pasteButton->setScale(0.5f);
        pasteButton->m_baseScale = 0.5f;
        menu->addChild(pasteButton);

        auto clearButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"), this, menu_selector(EditTextLayer::onClearText));
        clearButton->setPosition(inputBG->getPositionX() + 150, inputBG->getPositionY());
        clearButton->setScale(0.5f);
        clearButton->m_baseScale = 0.5f;
        menu->addChild(clearButton);
        
        return true;
    }

    void textChanged(CCTextInputNode* p0) {
        auto fields = m_fields.self();
        auto str = p0->getString();
        // p sure geode has a util for this that ive literally used before but its fiiiiiiiine
        int pos = 0;
        while ((pos = str.find(fields->lineShortcut, pos)) != std::string::npos) {
            str.replace(pos, fields->lineShortcut.length(), "\n");
            pos += 1;
        }
        p0->setString(str);
        CustomizeObjectLayer::textChanged(p0);
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

    void openTextMenu(float dt) {
        if (auto button = m_textButton) button->activate();
    }
};