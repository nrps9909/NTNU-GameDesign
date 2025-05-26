#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional> // For std::function (if GameObject.hpp uses it)

#include <glm/glm.hpp>

// Forward declarations
class GameObject; // Assumed to be defined in GameObject.hpp
class Scene;      // Assumed to be defined in Scene.hpp
struct GLFWwindow; 

// Dialog System Enums and Structs
inline enum class DialogChoice { non, A, B, C } dialogChoice;

enum class DialogType { DIALOG, QUIZ, GOODEND, BADEND };

struct DialogBase {
	DialogType type;
	DialogBase(DialogType type) : type(type) {}
	virtual ~DialogBase() = default;
};

struct Dialog : DialogBase {
	std::vector<std::string> lines;
	Dialog() : DialogBase(DialogType::DIALOG) {}
};

struct Quiz : DialogBase {
	std::string question;
	std::vector<std::string> options;
	int ansIndex = -1;
	mutable int userIndex = -1; // Mutable to allow modification in const renderQuiz
	std::vector<int> v_score;
	std::vector<std::string> feedback;
	Quiz() : DialogBase(DialogType::QUIZ) {}
};

struct NPC {
	std::shared_ptr<GameObject> go;
	std::vector<std::shared_ptr<DialogBase>> dialogs;
	bool routeEnabled{false};
	bool showIcon{false};
	bool inDialog{false};
	size_t scriptIndex{0};
	size_t lineIndex{0};
	int totalScore{0};
	bool isPlayingIdleAnimation{false};
	float idleAnimationTime{0.0f};
	int idleAnimationIndex{-1};
};

// DialogSystem Class Declaration
class DialogSystem {
public:
	static DialogSystem& getInstance();

	NPC& addNPC(std::shared_ptr<GameObject> go, std::vector<std::shared_ptr<DialogBase>> script);

	void update(Scene& scene, float dt);
	void render(Scene const& scene);
	void processInput(GLFWwindow* window);

    // MOVED or ensured to be public
    int findIdleAnimationIndex(std::shared_ptr<GameObject> const& go);

private:
	DialogSystem() = default;

	static glm::vec2 worldToScreen(glm::vec3 const& pos, Scene const& scene, int viewportW, int viewportH);

	void renderDialogUI();
	void renderDialog(Dialog const& dialog, NPC& npc);
	void renderQuiz(Quiz const& quiz, NPC& npc);
	void renderEnding(Dialog const& ending, NPC& npc, bool isGoodEnding);

	void handleDialogProgress(NPC& npc);

	void initializeNPCIdleAnimation(NPC& npc);
	void updateNPCIdleAnimation(NPC& npc, float dt);
	void startIdleAnimation(NPC& npc);
	// int findIdleAnimationIndex(std::shared_ptr<GameObject> const& go); // Removed from private if it was here

	std::vector<NPC> npcs_;
};

// To make the inline init functions compile within this header,
// GameObject must be fully defined.
#include "GameObject.hpp" 

// initBegin, initA, initB, initC function definitions
inline void initBegin(std::shared_ptr<GameObject> go)
{
	std::shared_ptr<Dialog> d1 = std::make_shared<Dialog>();
	d1->lines.push_back("（你原本想選修一門正常的課，走進分部圖書館後，看向手機確認選課系統時，發現自己莫名多了一門「遊戲程式與戀愛學特訓班」。）");
	d1->lines.push_back("（正準備點開查看時，突然被一隻有力的大手扯住袖子——）");
	d1->lines.push_back("老師：「割布麟同學，請問你母單嗎？」");
	d1->lines.push_back("割布麟：「......？」");
	d1->lines.push_back("老師：「我想...這門課應該很適合你，趕快進來吧！」");
	d1->lines.push_back("割布麟：「等、等等——這門課到底是什麼？！它根本不在課表裡啊！」");
	d1->lines.push_back("（老師神秘地推了推眼鏡，教室大門自動在割布麟身後關上。）");
	d1->lines.push_back("老師：「這是一門結合 AI、遊戲設計和……戀愛學的終極課程，你的任務很簡單——成功攻略我設計的 AI 角色，否則……直接不及格。」");
	d1->lines.push_back("割布麟：「什麼？！可是我母單20年！！！這太強人所難了吧」");
	d1->lines.push_back("老師（微笑）：「不會吧，連 NPC 都談不下來？」");
	d1->lines.push_back("（...）");

	std::shared_ptr<Quiz> q1 = std::make_shared<Quiz>();
	q1->question = "老師：「戀愛遊戲的核心是角色設計！來吧，為你的 AI 角色設計一個迷人的設定！」";
	q1->options.push_back("1.「應該有一個強烈的背景故事，讓角色有層次感！」");
	q1->options.push_back("2.「當然要有甜蜜的戀愛情節，製造心動瞬間！」");
	q1->options.push_back("3.「沉浸式互動才是王道，讓玩家自由選擇情節發展！」");

	std::shared_ptr<Dialog> d2 = std::make_shared<Dialog>();
	d2->lines.push_back("老師：「很好，現在，讓你的 AI 角色開始對話吧！」");
	d2->lines.push_back("（割布麟開始體驗第一場 AI 模擬對話，但……）");
	d2->lines.push_back("AI 角色：「初次見面……請輸入選項……」");
	d2->lines.push_back("（系統錯誤，AI 角色突然開始胡言亂語）");
	d2->lines.push_back("AI 角色：「這不是約會，而是統計數據的美妙運算！」");
	d2->lines.push_back("割布麟：「老師，這個 AI 真的能攻略嗎？！」");
	d2->lines.push_back("老師（推眼鏡）：「那就要看你的能力了。」");
	d2->lines.push_back("（進入決定初始好感度劇情，玩家選擇回應方式將決定分數）");

	std::shared_ptr<Quiz> q2 = std::make_shared<Quiz>();
	q2->question = "1. 心儀對象跟你說想出門看最近最流行的玫瑰園，你會穿什麼？";
	q2->options.push_back("A. 簡單的黑白灰格紋襯衫");
	q2->v_score.push_back(0);
	q2->options.push_back("B. 平常手臂有加強，穿高磅素T就好");
	q2->v_score.push_back(10);
	q2->options.push_back("C. GU 大地色穿搭，短褲白襪");
	q2->v_score.push_back(5);

	std::shared_ptr<Quiz> q3 = std::make_shared<Quiz>();
	q3->question = "2. 朋友揪去夜店玩，你的第一個反應是？";
	q3->options.push_back("A.「蛤？那邊不是很貴嗎？」");
	q3->v_score.push_back(0);
	q3->options.push_back("B.「誒剛好！可以揪認識的脆友在夜店見面」");
	q3->v_score.push_back(10);
	q3->options.push_back("C.「好啊。我常去。（結果回去偷偷焦慮襯衫會不會太正式。）」");
	q3->v_score.push_back(5);

	std::shared_ptr<Quiz> q4 = std::make_shared<Quiz>();
	q4->question = "3. 你正在用交友軟體，突然滑到一個超對你胃口的女生，你的開場白是？";
	q4->options.push_back("A.「我也喜歡這部電影！」");
	q4->v_score.push_back(5);
	q4->options.push_back("B.「哈哈哈哈哈」");
	q4->v_score.push_back(10);
	q4->options.push_back("C.「嗨～尼看起來豪有氣質，平常喜歡看書嗎？」");
	q4->v_score.push_back(0);

	std::shared_ptr<Quiz> q5 = std::make_shared<Quiz>();
	q5->question = "4. 你喜歡的女生說最近壓力好大，想要來點小確幸，你的選擇是？";
	q5->options.push_back("A.「記得你上次發限動想看夜景？今天晚上我開車載你去陽明山呀」");
	q5->v_score.push_back(10);
	q5->options.push_back("B.「晚上送宵夜給你呀，你想吃什麼？」");
	q5->v_score.push_back(0);
	q5->options.push_back("C.「帶你去吃我家巷口的火鍋店！」");
	q5->v_score.push_back(5);

	std::shared_ptr<Quiz> q6 = std::make_shared<Quiz>();
	q6->question = "5. 你長得如何？（誠實回答！）";
	q6->options.push_back("A.「長得普通啦，反正看順眼最重要。」");
	q6->v_score.push_back(0);
	q6->options.push_back("B.「師大彭于晏」");
	q6->v_score.push_back(10);
	q6->options.push_back("C.「還可以啦，有時候會被說耐看。」");
	q6->v_score.push_back(5);

	std::shared_ptr<Quiz> q7 = std::make_shared<Quiz>();
	q7->question = "6. 女生問：「你 IG版面怎麼都沒發文？」你會怎麼回答？";
	q7->options.push_back("A.「懶得發，而且生活沒什麼特別的。」");
	q7->v_score.push_back(0);
	q7->options.push_back("B.「我都典藏了啦，沒什麼人在看。」");
	q7->v_score.push_back(5);
	q7->options.push_back("C.「哈哈我都發摯友啦，等下加妳進去。」");
	q7->v_score.push_back(10);

	std::shared_ptr<Quiz> q8 = std::make_shared<Quiz>();
	q8->question = "7. 你的身高是？（誠實回答！）";
	q8->options.push_back("A.「178，剛好不超標！」");
	q8->v_score.push_back(5);
	q8->options.push_back("B.「182，不過應該還好吧？」");
	q8->v_score.push_back(10);
	q8->options.push_back("C.「170，這題對我很友善。」");
	q8->v_score.push_back(0);

	std::shared_ptr<Quiz> q9 = std::make_shared<Quiz>();
	q9->question = "8. 女生突然說：「你覺得男生應該主動付錢嗎？」你的反應？";
	q9->options.push_back("A.「AA 最公平吧？」");
	q9->v_score.push_back(0);
	q9->options.push_back("B.「當然要付啊，小錢啦」");
	q9->v_score.push_back(10);
	q9->options.push_back("C.「要看關係啦，曖昧的話請一下也 OK 吧？」");
	q9->v_score.push_back(5);

	std::shared_ptr<Quiz> q10 = std::make_shared<Quiz>();
	q10->question = "9. 她要過生日，你會送什麼？";
	q10->options.push_back("A.「送手作的禮物比較有心意吧？」");
	q10->v_score.push_back(0);
	q10->options.push_back("B.「送香水組合，之後再問她喜歡哪個味道」");
	q10->v_score.push_back(10);
	q10->options.push_back("C.「買個可愛的蛋糕小加手寫卡片。」");
	q10->v_score.push_back(5);

	std::shared_ptr<Quiz> q11_initial = std::make_shared<Quiz>();
	q11_initial->question = "10. 你有沒有女朋友？";
	q11_initial->options.push_back("A.「沒有，之前追過但沒成功。」");
	q11_initial->v_score.push_back(0);
	q11_initial->options.push_back("B.「有過幾個，但現在單身。」");
	q11_initial->v_score.push_back(10);
	q11_initial->options.push_back("C.「剛被分手，但我還沒走出來。」");
	q11_initial->v_score.push_back(5);

	std::shared_ptr<Dialog> transitionDialog = std::make_shared<Dialog>();
	transitionDialog->lines.push_back("老師：「很好！現在讓我們看看你的哥布林指數...」");
	transitionDialog->lines.push_back("（系統正在計算你的分數...）");
	transitionDialog->lines.push_back("老師：「根據你的回答，我為你安排了最適合的AI角色進行攻略練習。」");
	transitionDialog->lines.push_back("老師：「請選擇你想要學習的課程方向：」");

	std::shared_ptr<Quiz> characterSelection = std::make_shared<Quiz>();
	characterSelection->question = "選擇你的學習路線：";
	characterSelection->options.push_back("A. 程式邏輯導向 - 周理安（行為樹AI設計）");
	characterSelection->options.push_back("B. 創意劇本導向 - 林夢瑤（戀愛劇情設計）");
	characterSelection->options.push_back("C. 心理分析導向 - 沈奕恆（情感互動設計）");
	characterSelection->v_score = {0, 0, 0}; // This choice doesn't affect score, only determines route

	NPC& npc = DialogSystem::getInstance().addNPC(go, {
		d1, q1, d2, q2, q3, q4, q5, q6, q7, q8, q9, q10, q11_initial, 
		transitionDialog, characterSelection
	});
	npc.routeEnabled = true;
    if (npc.go) npc.go->invMass = 0;
	npc.inDialog = false; 
}

inline void initA(std::shared_ptr<GameObject> go) //周理安 - 行為樹AI
{
	std::shared_ptr<Dialog> d11 = std::make_shared<Dialog>();
	d11->lines.push_back("（場景：圖書館 801 教室，課堂開始，螢幕正播放「什麼是行為樹 AI」的簡報動畫）");
	d11->lines.push_back("周理安：「遊戲的核心是演算法與機制，而不是表面的情感渲染。」");
	d11->lines.push_back("老師：「很好，現在我們來設計 NPC 的行為樹，讓角色能根據玩家的選擇產生不同的對話與反應。」");
	d11->lines.push_back("主角（murmur）：「這門課的內容……怎麼越來越像演算法的學習課程了？」");
	d11->lines.push_back("老師：「要攻略 AI，首先你得思考：如果 NPC 有思考能力，它會根據什麼改變行為？」");
	d11->lines.push_back("主角：「這比攻略活人還難吧……」");
	d11->lines.push_back("周理安（推了推眼鏡）：「思考要條理、邏輯要清晰——不然連 ‘if’ 條件都判斷不了。」");

	std::shared_ptr<Dialog> d12 = std::make_shared<Dialog>();
	d12->lines.push_back("周理安：「我們來談談什麼是行為樹。」");
	d12->lines.push_back("（教室裡，老師拿出一塊寫滿愛心與箭頭的白板。）");
	d12->lines.push_back("老師：「戀愛不是亂槍打鳥，是有策略的行為流程。行為樹就是一種用來安排行為順序的結構——像是戀愛流程圖！」");
	d12->lines.push_back("周理安：「根節點就是起點，從這裡開始分析你的戀愛流程。」");
	d12->lines.push_back("周理安：「簡單來說，行為樹從『根節點』開始，下方是『控制節點』與『行為節點』。執行會從上往下，一步步判斷。」");

	std::shared_ptr<Quiz> q11 = std::make_shared<Quiz>();
	q11->question = "周理安：「測驗開始。你第一次傳訊息給喜歡的人時，哪個最像『行為樹的根節點』？」";
	q11->options.push_back("A. 說晚安");
	q11->options.push_back("B. 確認對方有沒有上線");
	q11->options.push_back("C. 決定要不要傳訊息");
	q11->options.push_back("D. 看對方的限時動態");
	q11->v_score = {5, 5, 0, 5};
	q11->ansIndex = 2;
    q11->feedback.resize(4);
    q11->feedback[0] = "周理安：「這是行為，但不是起始決策。」";
    q11->feedback[1] = "周理安：「這是條件檢查，但還不是根源。」";
    q11->feedback[2] = "周理安：「正確。一切行動始於決策。\n主角（心想）：「原來戀愛也有 if-else 條件判斷啊……」」";
    q11->feedback[3] = "周理安：「觀察是過程，但根節點是更早的決策。」";

	std::shared_ptr<Quiz> q12 = std::make_shared<Quiz>();
	q12->question = "周理安：「下一個問題。哪個說法最接近行為樹的『從上往下、從左到右執行』的特性？」";
	q12->options.push_back("A. 先看對方限動再決定行動");
	q12->options.push_back("B. 同時去對方家門口、教室門口、IG留言");
	q12->options.push_back("C. 先告白再看對方長怎樣");
	q12->options.push_back("D. 隨便點一個選項看運氣");
	q12->v_score = {0, 10, 10, 5};
	q12->ansIndex = 0;
    q12->feedback.resize(4);
    q12->feedback[0] = "周理安：「是的，這體現了順序性。\n周理安：「邏輯比衝動重要。這是基本。」」";
    q12->feedback[1] = "周理安：「行為樹通常是循序執行，而非並行。」";
    q12->feedback[2] = "周理安：「順序錯了，這不符合邏輯流程。」";
    q12->feedback[3] = "周理安：「行為樹講求的是明確的邏輯，不是隨機。」";


	std::shared_ptr<Dialog> d21 = std::make_shared<Dialog>();
	d21->lines.push_back("周理安：「接下來，我們討論 Selector 和 Sequence 節點。」");
	d21->lines.push_back("（走廊上，理安遞給你一張便條紙。）");
    d21->lines.push_back("周理安：「這是戀愛流程的兩種邏輯模型。看懂再說話。」");
	d21->lines.push_back("周理安：「Selector，選擇節點，像是『今天邀約的方式』。如果約喝咖啡失敗，就嘗試約吃拉麵，再失敗就試約看書。只要一個成功，整個選擇就成功並停止，像是在嘗試不同方法。」");
	d21->lines.push_back("周理安：「Sequence，序列節點，像是『告白前的準備流程』。要確保：對方心情好、自己沒口臭、場地氣氛OK，所有條件都成功，才能執行最終的『告白』動作。任何一步失敗，整個序列就失敗。」");

	std::shared_ptr<Quiz> q21 = std::make_shared<Quiz>();
	q21->question = "周理安：「測驗。你要跟我告白，哪個是 Sequence 的例子？」";
	q21->options.push_back("A. 直接告白失敗了就跑走");
	q21->options.push_back("B. 確認我在、準備花、深呼吸、才走過去");
	q21->options.push_back("C. 同時拿三束花丟給三個人看誰接");
	q21->options.push_back("D. 靠直覺衝過去喊「我喜歡你」");
	q21->v_score = {0, 10, 5, 0}; 
	q21->ansIndex = 1;
    q21->feedback.resize(4);
    q21->feedback[0] = "周理安：「這更像單一行為及其後果，不是序列。」";
    q21->feedback[1] = "周理安：「正確。這描述了一系列必須依次成功的步驟。\n主角（心想）：「感覺像在寫 SOP……戀愛還真嚴謹。」」";
    q21->feedback[2] = "周理安：「這聽起來很混亂，不符合序列的有序性。」";
    q21->feedback[3] = "周理安：「衝動行事，缺乏序列要求的步驟檢查。」";

	std::shared_ptr<Quiz> q22 = std::make_shared<Quiz>();
	q22->question = "周理安：「Selector 比喻成戀愛狀況，最接近哪個？」";
	q22->options.push_back("A. 告白一定要成功，不然整個流程停止");
	q22->options.push_back("B. 今天一定要約成，不管用什麼方法");
	q22->options.push_back("C. 失敗一次就放棄");
	q22->options.push_back("D. 每個條件都要達成才能告白");
	q22->v_score = {5, 10, 0, 5}; 
	q22->ansIndex = 1;
    q22->feedback.resize(4);
    q22->feedback[0] = "周理安：「這是 Sequence 中途失敗的結果，不是 Selector 的特性。」";
    q22->feedback[1] = "周理安：「對。Selector 會嘗試所有子節點直到一個成功為止。\n周理安：「會變通的人，戀愛才有機會。」」";
    q22->feedback[2] = "周理安：「Selector 會嘗試所有選項，直到成功或所有都失敗。這太快放棄了。」";
    q22->feedback[3] = "周理安：「這是 Sequence 的特性，要求所有條件都滿足。」";

	std::shared_ptr<Dialog> d31 = std::make_shared<Dialog>();
	d31->lines.push_back("周理安：「再來講講 Action，行為節點。」");
	d31->lines.push_back("（你終於鼓起勇氣問理安：『那角色實際上怎麼做事情？』她翻開一本筆記。）");
	d31->lines.push_back("理安：「葉子節點就是具體動作，比如走向某人、打招呼、送花。這些動作才會真的發生在遊戲中。」");
	d31->lines.push_back("理安：「記住，控制節點只是『流程管控』，Action 才是『真的執行』。」");

	std::shared_ptr<Quiz> q31 = std::make_shared<Quiz>();
	q31->question = "周理安：「下列哪一個最像是 Action 節點？」";
	q31->options.push_back("A. 思考是否要送花");
	q31->options.push_back("B. 規劃今天的行程");
	q31->options.push_back("C. 真正遞出那一束花");
	q31->options.push_back("D. 猶豫要不要傳訊息");
	q31->v_score = {5, 5, 10, 0}; 
	q31->ansIndex = 2;
    q31->feedback.resize(4);
    q31->feedback[0] = "周理安：「思考是內部過程，Action 是外部行為。」";
    q31->feedback[1] = "周理安：「規劃更像是控制節點的工作，決定行為順序。」";
    q31->feedback[2] = "周理安：「是的，這是具體的、可執行的動作。\n主角（心想）：「光想不行，還是得遞出花的那一刻才是真正的行動！」」";
    q31->feedback[3] = "周理安：「猶豫是狀態，不是執行的動作。」";

	std::shared_ptr<Quiz> q32 = std::make_shared<Quiz>();
	q32->question = "周理安：「你設計一個 NPC，當他看到喜歡的人時會『笑』這個行為，這是什麼？」";
	q32->options.push_back("A. 控制節點");
	q32->options.push_back("B. Sequence");
	q32->options.push_back("C. Action 節點");
	q32->options.push_back("D. 根節點");
	q32->v_score = {5, 5, 10, 5}; 
	q32->ansIndex = 2;
    q32->feedback.resize(4);
    q32->feedback[0] = "周理安：「控制節點決定流程，不直接執行『笑』。」";
    q32->feedback[1] = "周理安：「Sequence 是一連串動作，『笑』是單個動作。」";
    q32->feedback[2] = "周理安：「正確。『笑』是一個具體的行為。\n周理安：「角色不笑，你就沒有機會了。」」";
    q32->feedback[3] = "周理安：「根節點是整個行為樹的起點。」";

	std::shared_ptr<Dialog> d41 = std::make_shared<Dialog>();
	d41->lines.push_back("周理安：「現在來談談成功與失敗，Success/Failure。」");
	d41->lines.push_back("（你問理安：「如果我遞花她沒接呢？」）");
	d41->lines.push_back("周理安（淡淡說）：「那就是失敗。行為樹每一步都會回報『成功』或『失敗』，這會影響整體流程能不能繼續下去。」");
	d41->lines.push_back("周理安：「簡單說，行為節點會回傳『Success』或『Failure』。控制節點根據這些回傳值決定是否繼續下一步。」");

	std::shared_ptr<Quiz> q41 = std::make_shared<Quiz>();
	q41->question = "周理安：「測驗。你試圖讓 NPC 說「我喜歡你」，但對方角色不在現場。這個行為的回傳是？ 」";
	q41->options.push_back("A. Success");
	q41->options.push_back("B. Failure");
	q41->options.push_back("C. Running");
	q41->options.push_back("D. Happy");
	q41->v_score = {5, 10, 5, 0}; 
	q41->ansIndex = 1;
    q41->feedback.resize(4);
    q41->feedback[0] = "周理安：「目標未達成，不能算 Success。」";
    q41->feedback[1] = "周理安：「是的，前提條件不滿足，行為失敗。\n主角（心想）：「所以……這段戀愛判定失敗 Q_Q」」";
    q41->feedback[2] = "周理安：「Running 表示執行中，但這裡行為無法開始。」";
    q41->feedback[3] = "周理安：「Happy 不是行為樹的標準回傳狀態。」";

	std::shared_ptr<Quiz> q42 = std::make_shared<Quiz>();
	q42->question = "周理安：「在 Sequence 中，第二步驟失敗了，後面的行為還會執行嗎？」";
	q42->options.push_back("A. 一定會");
	q42->options.push_back("B. 不會");
	q42->options.push_back("C. 會視心情決定");
	q42->options.push_back("D. 看遊戲設定");
	q42->v_score = {5, 10, 0, 5}; 
	q42->ansIndex = 1;
    q42->feedback.resize(4);
    q42->feedback[0] = "周理安：「Sequence 要求所有步驟成功。一步失敗則整體失敗。」";
    q42->feedback[1] = "周理安：「正確。Sequence 的特性就是這樣。\n周理安：「戀愛流程中出現漏洞，當然得中止重來。」」";
    q42->feedback[2] = "周理安：「行為樹是依賴邏輯，不是心情。」";
    q42->feedback[3] = "周理安：「這是行為樹標準定義的一部分，不是隨意設定的。」";

	std::shared_ptr<Dialog> d51 = std::make_shared<Dialog>();
	d51->lines.push_back("周理安：「最後是 Running，執行中狀態。」");
	d51->lines.push_back("（某天下課後，你試著模擬一段 NPC 和玩家互動的劇情給理安看。）");
    d51->lines.push_back("周理安（點頭）：「你少了一個關鍵狀態：Running。」");
	d51->lines.push_back("周理安：「有些行為不是立即成功或失敗，而是正在進行中，例如等待回覆或角色移動。這種狀態就叫做 Running。」");

	std::shared_ptr<Quiz> q51 = std::make_shared<Quiz>();
	q51->question = "周理安：「你傳訊息後，對方已讀但還沒回，這是哪種狀態？」";
	q51->options.push_back("A. Success");
	q51->options.push_back("B. Failure");
	q51->options.push_back("C. Running");
	q51->options.push_back("D. Timeout");
	q51->v_score = {5, 0, 10, 5}; 
	q51->ansIndex = 2;
    q51->feedback.resize(4);
    q51->feedback[0] = "周理安：「還沒收到回覆，不能算成功。」";
    q51->feedback[1] = "周理安：「雖然可能讓人焦慮，但技術上還未失敗。」";
    q51->feedback[2] = "周理安：「對。等待回應就是一種典型的 Running 狀態。\n主角（心想）：「這才是真正最折磨人的狀態……戀愛中的 loading 畫面。」」";
    q51->feedback[3] = "周理安：「Timeout 可能是 Failure 的一種原因，但 Running 是當前狀態。」";

	std::shared_ptr<Quiz> q52 = std::make_shared<Quiz>();
	q52->question = "周理安：「NPC 開始走向喜歡的人，中途還沒走到，屬於什麼狀態？」";
	q52->options.push_back("A. Failure");
	q52->options.push_back("B. Waiting");
	q52->options.push_back("C. Running");
	q52->options.push_back("D. Ending");
	q52->v_score = {0, 5, 10, 5}; 
	q52->ansIndex = 2;
    q52->feedback.resize(4);
    q52->feedback[0] = "周理安：「除非中途有障礙無法到達，否則還不是 Failure。」";
    q52->feedback[1] = "周理安：「Waiting 太籠統，Running 更精確描述進行中的動作。」";
    q52->feedback[2] = "周理安：「是的，移動過程是持續性的，屬於 Running。\n周理安：「在愛情裡，進行中的動作，也是一種希望。」」";
    q52->feedback[3] = "周理安：「還沒到結局呢。」";

	std::shared_ptr<Dialog> e1 = std::make_shared<Dialog>(); // Good end for initA
	e1->type = DialogType::GOODEND;
	e1->lines.push_back("（夕陽下，你與理安一起站在天台邊緣，風輕輕吹起她的頭髮。）");
	e1->lines.push_back("周理安（低聲）：「你居然……真的學會了全部的行為樹邏輯？就連 Running 的邏輯都能用來比喻等喜歡的人回訊息……」");
	e1->lines.push_back("你：「我為了能和妳說上話，特訓了好幾天。」");
	e1->lines.push_back("（她輕輕瞪了你一眼，然後眼神轉為柔和。）");
	e1->lines.push_back("周理安：「那我現在的狀態是什麼？」");
	e1->lines.push_back("你（盯著她的眼睛）：「應該是……Running，因為我還不知道你對我的回應。」");
	e1->lines.push_back("周理安（停頓）：「錯了，是 Success，你這笨蛋。」");
	e1->lines.push_back("「你成功通關了《遊戲程式與戀愛學特訓班》：周理安路線｜攻略達成」");

	std::shared_ptr<Dialog> e2 = std::make_shared<Dialog>(); // Bad end for initA
	e2->type = DialogType::BADEND;
	e2->lines.push_back("（空蕩蕩的教室，結課的最後一晚。）");
	e2->lines.push_back("（你坐在位子上，看著空空如也的白板。桌上放著你的測驗結果——答錯了太多題。）");
	e2->lines.push_back("老師（拍你肩膀）：「不錯了，至少你撐到最後。不過這堂課不是誰都能順利通關的。」");
	e2->lines.push_back("（你低頭一笑，望向窗外。）");
	e2->lines.push_back("主角（murmur）：「原來……就算懂了一堆理論，戀愛還是不能全靠演算法。」");
	e2->lines.push_back("（這時，門口傳來熟悉的腳步聲。）");
	e2->lines.push_back("周理安：「……你不及格了耶。」");
	e2->lines.push_back("你：「對啊，我猜我沒辦法用行為樹攻略你了。」");
	e2->lines.push_back("（她站在門邊，忽然露出一點笑意。）");
	e2->lines.push_back("周理安：「那就……改用別的演算法再試一次啊。」");
	e2->lines.push_back("（畫面轉黑，顯示文字）");
	e2->lines.push_back("「你未能通關《遊戲程式與戀愛學特訓班》：周理安路線｜未攻略成功，但故事還沒結束……？」");

	NPC& character_npc = DialogSystem::getInstance().addNPC(go, {
        d11, d12, q11, q12, d21, q21, q22, d31, q31, q32, 
        d41, q41, q42, d51, q51, q52, e1, e2
    });
	if (character_npc.go) character_npc.go->invMass = 0;
	character_npc.inDialog = false; 
	character_npc.routeEnabled = true; 
}

inline void initB(std::shared_ptr<GameObject> go) // 林夢瑤 - 創意劇本導向
{
	std::shared_ptr<Dialog> d11 = std::make_shared<Dialog>();
	d11->lines.push_back("（你剛剛坐下，就被一疊粉紅色的劇本砸到。）");
	d11->lines.push_back("林夢瑤（驚呼）：「啊！對不起對不起！我剛剛想測試拋物線軌跡的感覺，沒想到砸到人了！」");
	d11->lines.push_back("主角：「拋物線……劇本……？這門課到底是來上程式的還是來拍偶像劇的？」");
	d11->lines.push_back("（此時，劉焱成老師大聲宣布：）");
	d11->lines.push_back("老師：「今天開始，我們將進入《戀愛路線模擬與情感選擇架構》模組。你們要做的，就是設計一款讓玩家心跳加速、愛到卡慘死的遊戲。」");
	d11->lines.push_back("林夢瑤（雙眼閃亮）：「這不就是我一直夢想的那種、會讓人忘記現實的戀愛世界嗎？」");
	d11->lines.push_back("（你猶豫了片刻，卻又無法抗拒她的熱情邀請，一起踏上這條粉紅泡泡的學習路線……）");

	std::shared_ptr<Dialog> d12 = std::make_shared<Dialog>();
	d12->lines.push_back("林夢瑤：「來學習戀愛劇本的心跳公式——情節張力與起承轉合！」");
	d12->lines.push_back("（圖書館801教室，一張堆滿粉紅便條紙的白板上，寫滿了各種劇情模板。林夢瑤手拿馬克筆，正熱血沸騰地畫出愛心箭頭與戀愛三角。）");
	d12->lines.push_back("林夢瑤：「一個讓人上癮的戀愛劇情，不能只是男主遞衛生紙給女主就感天動地好嗎～要有『情緒張力』！有衝突、有誤會、有心動才有價值！這些都要安排在劇本的起承轉合裡！」");
	d12->lines.push_back("主角（murmur）：「這堂課是戀愛心理還是結構寫作⋯⋯？」");
	d12->lines.push_back("林夢瑤：「不！這是心跳設計學！『起』要有獨特邂逅；『承』要鋪陳日常互動；『轉』得來個衝突或誤會；『合』則是高潮與情感昇華。太平凡的戀愛，只會讓玩家點右上角退出。」");
	d12->lines.push_back("（螢幕上彷彿出現了圖解：「戀愛劇本四階段」的字樣。）");

	std::shared_ptr<Quiz> q11 = std::make_shared<Quiz>();
	q11->question = "林夢瑤：「Q1：哪一個事件最適合安排在『轉』的階段？」";
	q11->options.push_back("A. 男主遞早餐給女主");
	q11->options.push_back("B. 女主誤會男主和青梅竹馬交往");
	q11->options.push_back("C. 男女主角在圖書館第一次相遇");
	q11->options.push_back("D. 兩人互許心願去看流星雨");
	q11->v_score = {5, 10, 5, 0}; 
	q11->ansIndex = 1;
    q11->feedback.resize(4);
    q11->feedback[0] = "林夢瑤：「這個比較平淡，適合放在『承』喔。」";
    q11->feedback[1] = "林夢瑤：「賓果！『轉』就是要這種衝突和誤會，才能讓玩家揪心又想看下去啊～」";
    q11->feedback[2] = "林夢瑤：「第一次相遇，當然是『起』點囉！」";
    q11->feedback[3] = "林夢瑤：「這個比較像『合』的部分，情感昇華的時刻。」";

	std::shared_ptr<Quiz> q12 = std::make_shared<Quiz>();
	q12->question = "林夢瑤：「Q2：以下哪個情節最適合當作『起』的開端？」";
	q12->options.push_back("A. 男女主角在社團吵架");
	q12->options.push_back("B. 男主為女主擋下掉落的書本");
	q12->options.push_back("C. 女主看到男主和別人牽手");
	q12->options.push_back("D. 女主悄悄觀察男主的社群帳號很久");
	q12->v_score = {5, 10, 5, 0}; 
	q12->ansIndex = 1;
    q12->feedback.resize(4);
    q12->feedback[0] = "林夢瑤：「吵架當開頭？也不是不行，但可能比較刺激一點，看你想寫什麼風格！」";
    q12->feedback[1] = "林夢瑤：「對嘛！這就是所謂的『瞬間命運感』！一個經典又百看不厭的邂逅方式！」";
    q12->feedback[2] = "林夢瑤：「這個比較像『轉』的劇情，製造誤會用的！」";
    q12->feedback[3] = "林夢瑤：「嗯...這個當作背景設定可以，但作為開場邂逅，戲劇性不太夠哦。」";

	std::shared_ptr<Dialog> d21 = std::make_shared<Dialog>();
	d21->lines.push_back("林夢瑤：「接下來是角色性格建構：從 MBTI 到反差萌！」");
	d21->lines.push_back("（隔天清晨，圖書館窗邊灑進一縷光，林夢瑤蹲在角落，一邊看著偶像劇的設定本，一邊激動地在筆記本上畫著人設表。主角靠近時，她突然轉頭，眼神閃爍。）");
	d21->lines.push_back("林夢瑤：「我昨天夢到一個超帥的反社會型男主，他外表冷酷，實際會偷偷幫女主撿掉在地上的補習班傳單……這種『反差』你懂嗎！超級重要的啦！」");
	d21->lines.push_back("主角（murmur）：「反社會型……撿傳單？這是什麼神奇組合……？」");
	d21->lines.push_back("林夢瑤：「來，我們先不講夢了，講理論！角色要有邏輯，但不能無聊！MBTI可以幫你建立角色骨架，但真正讓人愛上的，是那個出其不意的『反差』——比如冷面殺手也會怕蟑螂！」");
	d21->lines.push_back("（她突然舉起一個白板，上面畫了三個角色設定，並用愛心和爆炸圖標標註：）");
	d21->lines.push_back("林夢瑤：「例如，INFJ 看似沉默寡言，但可能私下寫的日記會充滿十頁戀愛妄想。」");
	d21->lines.push_back("林夢瑤：「或者 ENTP，話多又跳Tone，但私底下可能默默做著超細緻的便當。」");
	d21->lines.push_back("林夢瑤：「還有 ISTP，外表冷靜理性，但對戀愛可能毫無經驗，只會模仿電影橋段告白。」");
	d21->lines.push_back("林夢瑤：「看出來了嗎？不是MBTI定一切，而是你怎麼在既有框架裡製造驚喜！這樣角色才會有人氣嘛～」");

	std::shared_ptr<Quiz> q21 = std::make_shared<Quiz>();
	q21->question = "林夢瑤：「Q3：以下哪一個是常見的「反差萌」設定？」";
	q21->options.push_back("A. 女主是溫柔體貼型，但其實擅長格鬥");
	q21->options.push_back("B. 男主是活潑型，經常搞笑又遲到");
	q21->options.push_back("C. 女主是害羞型，會迴避所有互動");
	q21->options.push_back("D. 男主是學霸，對感情毫無興趣");
	q21->v_score = {10, 5, 0, 5}; 
	q21->ansIndex = 0;
    q21->feedback.resize(4);
    q21->feedback[0] = "林夢瑤：「沒錯沒錯～反差就是你原本以為她只能溫柔，結果她一拳打飛流氓，這才叫讓人心動嘛！」";
    q21->feedback[1] = "林夢瑤：「這個比較像性格一致，反差感不夠強烈喔。」";
    q21->feedback[2] = "林夢瑤：「如果只是迴避，可能比較難發展劇情，反差感也不明顯。」";
    q21->feedback[3] = "林夢瑤：「這也是一種設定，但『反差』的驚喜感比較少。」";

	std::shared_ptr<Quiz> q22 = std::make_shared<Quiz>();
	q22->question = "林夢瑤：「Q4：哪個角色設定最有機會吸引喜歡「理性男」的玩家？」";
	q22->options.push_back("A. INFP，時常情緒波動，夢想成為詩人");
	q22->options.push_back("B. ESTJ，重視效率，會依照時間表談戀愛");
	q22->options.push_back("C. ISFP，喜歡自己一個人待在樹下發呆");
	q22->options.push_back("D. ENFP，每天都有新的戀愛理論想分享");
	q22->v_score = {5, 10, 0, 5}; 
	q22->ansIndex = 1;
    q22->feedback.resize(4);
    q22->feedback[0] = "林夢瑤：「INFP 的感性可能會吸引另一種玩家，但理性男可能比較喜歡條理分明的。」";
    q22->feedback[1] = "林夢瑤：「對啊！有些玩家就是吃這套『規則系戀愛』，而且越硬派越有反差潛力，比如他搞不好還會做愛情Excel表格呢！」";
    q22->feedback[2] = "林夢瑤：「ISFP 喜歡獨處，可能比較難讓理性男感覺到互動的火花。」";
    q22->feedback[3] = "林夢瑤：「ENFP 的熱情很好，但過於發散的理論可能不是理性男的首選。」";

	std::shared_ptr<Dialog> d31 = std::make_shared<Dialog>();
	d31->lines.push_back("林夢瑤：「接下來的課題是，情緒是糖，節奏是鹽！」");
	d31->lines.push_back("（你與林夢瑤坐在圖書館八樓窗邊，外頭雨滴滴答敲著玻璃，她正翻閱一本標題是《讓你的主角哭得觀眾痛快》的戀愛劇本寫作書。你們要一起學習：如何設計角色的情緒曲線與劇情節奏。）");
	d31->lines.push_back("林夢瑤（雙眼閃亮）：「欸欸，你有發現嗎？所有讓人超級上頭的戀愛劇情——都會有那種『突然好甜！然後下一秒就虐爆』的轉折！你不覺得超帶感嗎？」");
	d31->lines.push_back("主角：「呃……帶感是什麼單位？」");
	d31->lines.push_back("林夢瑤：「拜託，戀愛的節奏就是要像糖鹽混著吃！不能一直甜，也不能一直虐——你要讓觀眾‘以為要親了結果掀桌’，才會尖叫啊～」");
	d31->lines.push_back("（林夢瑤遞給你一本自製筆記，封面還畫了可愛的愛心爆炸圖。）");
    d31->lines.push_back("林夢瑤：「你看，情緒曲線的基本原則是：角色必須經歷變化，不能從頭到尾都一樣開心或傷心。故事節奏要有張力，要有「推進-衝突-釋放」的節拍。最重要的是，給觀眾心理落差，才能產生情感參與！」");
    d31->lines.push_back("林夢瑤：「舉例來說，常見的戀愛節奏安排可以是：誤會 → 傷心 → 再相遇 → 心動 → 誤會再升級 → 大告白 → Happy End。或者更刺激的：突然親上去 → 被打 → 發現對方其實是間諜 → 邊逃亡邊談戀愛！」");
	d31->lines.push_back("林夢瑤：「總之你只要記住：讓角色有情緒曲線，觀眾才會投入啦嘿嘿～」");

	std::shared_ptr<Quiz> q31 = std::make_shared<Quiz>();
	q31->question = "林夢瑤：「Q5. 以下哪一組情緒曲線更容易讓玩家投入？」";
	q31->options.push_back("A. 主角從頭到尾都很開心，一路跟女主角打情罵俏，最後順利交往。");
	q31->options.push_back("B. 主角先討厭女主角→共患難→慢慢理解對方→產生情愫→突發衝突→最後和好。");
	q31->options.push_back("C. 主角一出場就大告白，然後開始甜蜜膩死人的生活。");
	q31->options.push_back("D. 主角一直傷心，女主角也沒出現，最後兩人都沒交集。");
	q31->v_score = {5, 10, 5, 0}; 
	q31->ansIndex = 1;
    q31->feedback.resize(4);
    q31->feedback[0] = "林夢瑤：「太順利了啦，少了點波折，玩家可能會覺得不夠深刻喔。」";
    q31->feedback[1] = "林夢瑤：「沒錯～因為它包含了「角色成長、情緒起伏、情節反轉」，是最符合情緒曲線設計原則的節奏！」";
    q31->feedback[2] = "林夢瑤：「進展太快了！沒有鋪陳的甜蜜，很容易膩的。」";
    q31->feedback[3] = "林夢瑤：「這樣太慘了啦，玩家會玩到心累的！」";

	std::shared_ptr<Quiz> q32 = std::make_shared<Quiz>();
	q32->question = "林夢瑤：「Q6. 如果你要讓觀眾在第六集開始瘋狂嗑糖，你應該在前幾集怎麼安排劇情？」";
	q32->options.push_back("A. 前五集完全沒互動，第六集直接接吻。");
	q32->options.push_back("B. 前幾集先鋪梗、互動冷淡、第六集突然有破防小舉動（例如意外牽手）。");
	q32->options.push_back("C. 前五集瘋狂灑糖，第六集也繼續曬恩愛。");
	q32->options.push_back("D. 第一集就親、第六集換另一個人親。");
	q32->v_score = {0, 10, 5, 5}; 
	q32->ansIndex = 1;
    q32->feedback.resize(4);
    q32->feedback[0] = "林夢瑤：「這樣太突然了，觀眾會跟不上情緒的！」";
    q32->feedback[1] = "林夢瑤：「是的！要讓觀眾感受到「進展」，就要先鋪墊反差，才會在第六集被甜到尖叫！」";
    q32->feedback[2] = "林夢瑤：「一直灑糖也不行啦，觀眾會麻木的，要有點起伏才刺激！」";
    q32->feedback[3] = "林夢瑤：「換人親？這劇情也太跳躍了吧！雖然...好像也蠻刺激的？」";

	std::shared_ptr<Dialog> d41 = std::make_shared<Dialog>();
	d41->lines.push_back("林夢瑤：「再來是角色語言風格與台詞設計！」");
	d41->lines.push_back("（圖書館 801教室中，林夢瑤正在拿出一本厚到可以當枕頭的《戀愛遊戲名場面語錄解析》。她一臉興奮地拍了拍你的肩膀）");
	d41->lines.push_back("林夢瑤：「你知道嗎？一個角色的靈魂，其實是藏在她說話的方式裡！講話沒特色，就像告白只說『我喜歡你』——會被當成詐騙訊息！」");
	d41->lines.push_back("（老師從遠方走來，突然一手抽出一張「戀愛語氣診斷表」，像魔法少女變身那樣撒出星光紙片）");
	d41->lines.push_back("劉焱成老師：「設計語言風格，就是設計一種人格的濾鏡！要讓每個角色講話時，玩家能用耳朵分辨出他們的靈魂濃度！」");
	d41->lines.push_back("主角（murmur）：「我昨天夢到自己講話講到被戀愛選項淹沒，最後只能靠一根吸管呼吸……」");
	d41->lines.push_back("林夢瑤（忘我地興奮接話）：「那我們來設計角色的語言人格——有的溫柔、有的傲嬌、有的裝酷、有的講話像機器人，讓每句台詞都能成為玩家截圖的動機！」");
	d41->lines.push_back("林夢瑤：「語言風格，簡單說就是根據角色背景、個性，設計符合角色語調的語句。一句話內盡量包含角色情緒與獨特表達方式。」");
    d41->lines.push_back("林夢瑤：「比方說，溫柔型可能會說：『如果你願意的話……可以陪我一下嗎？』");
    d41->lines.push_back("林夢瑤：「傲嬌型大概是：『才、才不是特地來找你的呢！』");
    d41->lines.push_back("林夢瑤：「搞怪型可能會說：『吃下這顆糖你就得娶我，這是……呃，糖果契約？』");
    d41->lines.push_back("林夢瑤：「機械風型角色則可能說：『資料分析中。感情異常感知提升27%。建議：心動。』這樣！」");

	std::shared_ptr<Quiz> q41 = std::make_shared<Quiz>();
	q41->question = "林夢瑤：「Q7. 傲嬌型女主角要對男主角告白，但她無法坦率表達。請選出最符合傲嬌語氣的告白句：」";
	q41->options.push_back("A.「我一直很喜歡你，從第一天就知道了。」");
	q41->options.push_back("B.「才、才不是特別想每天看到你啦，只是你太礙眼了啦笨蛋……」");
	q41->options.push_back("C.「你這樣讓我感覺很特別，我想和你試著交往看看。」");
	q41->options.push_back("D.「你今天還是這麼溫柔，像春天一樣。」");
	q41->v_score = {5, 10, 5, 0}; 
	q41->ansIndex = 1;
    q41->feedback.resize(4);
    q41->feedback[0] = "林夢瑤：「這個太直接了，不像傲嬌會說的話喔。」";
    q41->feedback[1] = "林夢瑤：「答對了！傲嬌的精髓就是口是心非，用否定句包裝真心！」";
    q41->feedback[2] = "林夢瑤：「這個比較像坦率型的告白，傲嬌通常更彆扭一點。」";
    q41->feedback[3] = "林夢瑤：「這是溫柔型或文學少女的台詞吧！」";

	std::shared_ptr<Quiz> q42 = std::make_shared<Quiz>();
	q42->question = "林夢瑤：「Q8. 你正在設計一個機器人女友角色，她會根據玩家互動變化語調。請選出最能代表她風格的句子：」";
	q42->options.push_back("A.「嘿，你又遲到了，我可是一直在等你。」");
	q42->options.push_back("B.「正在辨識……心率異常上升。資料庫標記為『喜歡』。」");
	q42->options.push_back("C.「你來啦～今天也要努力戀愛唷！」");
	q42->options.push_back("D.「不、不、不行這樣啦……我會害羞的 >///<」");
	q42->v_score = {5, 10, 5, 0}; 
	q42->ansIndex = 1;
    q42->feedback.resize(4);
    q42->feedback[0] = "林夢瑤：「這個比較像普通女友的抱怨，少了點機械感。」";
    q42->feedback[1] = "林夢瑤：「就是這個FEEL！機器人型角色常用理性邏輯來分析情感，超萌的！」";
    q42->feedback[2] = "林夢瑤：「這個比較像元氣少女的風格。」";
    q42->feedback[3] = "林夢瑤：「這是害羞內向型的角色吧，機器人女友通常更冷靜（表面上）。」";

	std::shared_ptr<Dialog> d51 = std::make_shared<Dialog>();
	d51->lines.push_back("林夢瑤：「最後一課：結局分歧與玩家影響設計！」");
	d51->lines.push_back("（夜深了，801教室只剩主角與林夢瑤兩人。窗外是靜謐的校園，螢光燈偶爾閃爍。林夢瑤一邊喝著便利商店買來的熱可可，一邊攤開她的戀愛遊戲企劃書。）");
	d51->lines.push_back("林夢瑤：「戀愛遊戲最迷人的地方，不只是誰跟誰在一起…而是『怎麼走到這裡的』。」");
	d51->lines.push_back("主角：「所以……玩家做的每個選擇，會導致不同的结局？」");
	d51->lines.push_back("林夢瑤：「對，這就像是人生分支模擬器，選擇愈多、愈能讓玩家感受到『我影響了這段感情』的重量。」");
	d51->lines.push_back("（老師突然從黑板後面探出頭，像幽靈一樣冷不防地插話：）");
	d51->lines.push_back("劉焱成老師：「結局設計分三種：感情成功 or 失敗，角色改變與否，玩家的自我感受。設計者要問自己一個問題——『結束時，角色和玩家都得到了什麼？』」");
	d51->lines.push_back("林夢瑤：「簡單講，就是讓『在一起』這件事，不是按鈕，而是一段旅程的證明。」");
	d51->lines.push_back("林夢瑤：「結局類型有很多，像是 True Ending，角色與玩家走到深度共鳴；Bad Ending，因選擇錯誤，感情破裂或角色黑化；還有 Neutral Ending，彼此尊重離開，保有好感但不交往。」");
    d51->lines.push_back("林夢瑤：「影響結局的方法，可以用『好感度』或『關鍵選擇』控制分歧。某些對話選項會累積分數，影響角色信任值。甚至可以設計隱藏選項與特定條件才解鎖的True Ending！」");
    d51->lines.push_back("林夢瑤：「情感回饋也很重要，結尾台詞要讓人記住，總結感情旅程。簡單的畫面演出，像煙火、牽手、角色消失等，都能加深記憶。」");

	std::shared_ptr<Quiz> q51 = std::make_shared<Quiz>();
	q51->question = "林夢瑤：「題目 9. 情境：你要設計一個結局，讓玩家因錯過所有重要選擇導致 Bad Ending，請問哪個選項最能符合「戀愛失敗但角色成長」的路線？」";
	q51->options.push_back("A. 角色決定轉學，兩人約定下次重逢時再重新開始");
	q51->options.push_back("B. 玩家在結尾時向角色告白成功，甜蜜牽手");
	q51->options.push_back("C. 角色消失在遊戲中，玩家找不到任何結局資訊");
	q51->options.push_back("D. 角色對玩家表示失望，並選擇離開，畫面漸黑");
	q51->v_score = {10, 5, 0, 5}; 
	q51->ansIndex = 0;
    q51->feedback.resize(4);
    q51->feedback[0] = "林夢瑤：「沒錯！這類結局雖未修成正果，但保有角色成長與未來可能性，是成熟的 Bad Ending 設計方式，不讓玩家有過度挫敗感。」";
    q51->feedback[1] = "林夢瑤：「這是 Good Ending 吧！題目是 Bad Ending 喔。」";
    q51->feedback[2] = "林夢瑤：「這樣玩家會很錯愕耶，連個交代都沒有！」";
    q51->feedback[3] = "林夢瑤：「這個比較像是純粹的失敗，角色成長的描寫比較少。」";

	std::shared_ptr<Quiz> q52 = std::make_shared<Quiz>();
	q52->question = "林夢瑤：「題目 10. 情境：你希望設計一個 True Ending，讓玩家覺得「這場戀愛值得一切努力」。下列哪個演出最有效？」";
	q52->options.push_back("A. 結尾兩人對話：「所以……我們現在，是戀人了嗎？」");
	q52->options.push_back("B. 畫面轉黑，只留下「感謝你玩到最後」字樣");
	q52->options.push_back("C. 兩人一起設計下一款戀愛遊戲，並暗示未來共事");
	q52->options.push_back("D. 角色給玩家一張紙條，上面寫著「再見」");
	q52->v_score = {5, 0, 10, 5}; 
	q52->ansIndex = 2;
    q52->feedback.resize(4);
    q52->feedback[0] = "林夢瑤：「這個不錯，確認關係是很重要的 момент！」";
    q52->feedback[1] = "林夢瑤：「呃，這個有點太敷衍了吧，True Ending 耶！」";
    q52->feedback[2] = "林夢瑤：「Bingo！這類結局不只表示戀愛成功，也讓雙方在目標上同步，呈現感情成長與共同前景，是理想 True Ending 設計。」";
    q52->feedback[3] = "林夢瑤：「『再見』？這聽起來比較像 Bad Ending 或 Neutral Ending 吧？」";

	std::shared_ptr<Dialog> e1 = std::make_shared<Dialog>(); // Good end for initB
	e1->type = DialogType::GOODEND;
	e1->lines.push_back("（美術系展演空間的角落，活動剛結束，你們坐在地板上，兩人靠得很近）");
	e1->lines.push_back("林夢瑤（輕聲）：「我一開始以為你只是想玩遊戲，沒想到你會陪我把整段劇情走完……還幫我補完那些我不敢寫的情節。」");
	e1->lines.push_back("（你笑了笑，手裡還拿著那份你們一起完成的腳本。）");
	e1->lines.push_back("主角：「因為那是我們兩個的故事，我不想讓它只停留在開頭。」");
	e1->lines.push_back("（林夢瑤垂下眼睫，似乎有點不好意思。）");
	e1->lines.push_back("林夢瑤：「那你覺得……我們的結局該怎麼寫？」");
	e1->lines.push_back("（你望向她的眼睛，語氣認真：）");
	e1->lines.push_back("主角：「這段劇情已經走到True Ending了，不用再選分支了。」");
	e1->lines.push_back("（她愣了一下，然後笑了。）");
	e1->lines.push_back("林夢瑤：「好，那我就把你寫進下一款遊戲裡……寫成一個讓我會心動的NPC。」");
	e1->lines.push_back("（畫面慢慢拉遠，燈光柔和，背景音樂響起）");
	e1->lines.push_back("「你成功通關了《遊戲程式與戀愛學特訓班》：林夢瑤路線｜攻略達成」");

	std::shared_ptr<Dialog> e2 = std::make_shared<Dialog>(); // Bad end for initB
	e2->type = DialogType::BADEND;
	e2->lines.push_back("（空教室，桌面上只剩一張被退件的企劃書，主角靜靜坐著翻閱。黑板上的日期，是課程結束的前一天。）");
	e2->lines.push_back("（你望著那份寫了一半的故事稿，裡面角色的對白停在一次爭吵之後，沒有结局。）");
	e2->lines.push_back("（老師的語音訊息在手機中播放——）");
	e2->lines.push_back("劉焱成老師：「劇本寫到最後，如果沒有情感支撐，那就是一堆流程而已。」");
	e2->lines.push_back("（你輕聲笑了一下，抬頭看著天花板。）");
	e2->lines.push_back("主角（murmur）：「果然還是太急了……想讓她喜歡上我，卻沒寫出她想要的劇情。」");
	e2->lines.push_back("（這時，教室門口傳來熟悉的聲音。）");
	e2->lines.push_back("林夢瑤（語氣平靜）：「我有看到你最近的設計，你有在進步。」");
	e2->lines.push_back("（你轉頭，她正站在門邊，身後是微弱的走廊燈。）");
	e2->lines.push_back("主角：「可是……我沒能幫你完成那個夢想的腳本。」");
	e2->lines.push_back("（林夢瑤沉默了一下，然後遞出一支隨身碟。）");
	e2->lines.push_back("林夢瑤：「那就留下來慢慢寫吧。不為通關，只是想和你……把故事寫完。」");
	e2->lines.push_back("（畫面慢慢轉暗，只剩窗邊微光）");
	e2->lines.push_back("「你未能通關《遊戲程式與戀愛學特訓班》：林夢瑤路線｜未攻略成功，但故事仍在繼續中……」");

	NPC& character_npc = DialogSystem::getInstance().addNPC(go, {
        d11, d12, q11, q12, d21, q21, q22, d31, q31, q32, 
        d41, q41, q42, d51, q51, q52, e1, e2
    });
	if (character_npc.go) character_npc.go->invMass = 0;
	character_npc.inDialog = false; 
	character_npc.routeEnabled = true; 
}

inline void initC(std::shared_ptr<GameObject> go) // 沈奕恆 - 心理分析導向
{
	std::shared_ptr<Dialog> d11 = std::make_shared<Dialog>();
	d11->lines.push_back("主角（murmur）：「蛤……這傢伙的邏輯也太哲學系了吧？」");
	d11->lines.push_back("老師：「很好！這位是沈奕恆，他將帶領你進入心理學導向的戀愛互動設計之路。」");
    d11->lines.push_back("沈奕恆：「開始第一個教學模組：角色視角的心理轉換是什麼？」");
	d11->lines.push_back("（教室燈光昏黃，沈奕恆正坐在最後一排，手裡翻著一本心理敘事學的書。你走進來，他抬頭看你一眼。）");
	d11->lines.push_back("老師（推開門，手裡抱著幾本厚重教材）：「今天我們不講遊戲機制，我們講『視角』——不只是從哪個角度看故事，而是誰在感受這段故事。」");
	d11->lines.push_back("（老師在白板上畫了兩個句子：）");
	d11->lines.push_back("（句子一：『他看到她哭了，有點不知所措。』）");
	d11->lines.push_back("（句子二：『我看到她哭了，心臟像是被擰了一下。』）");
	d11->lines.push_back("老師（轉頭問你）：「你比較想玩哪一個角色？」");
	d11->lines.push_back("沈奕恆（淡淡開口）：「第一句像在看別人談戀愛，第二句……像是我在戀愛。」");
	d11->lines.push_back("老師：「這就是第一人稱的魔力。」");
	d11->lines.push_back("（你愣了一下，試著低聲複誦：「我……心臟被擰了一下……」）");
	d11->lines.push_back("沈奕恆（輕笑）：「不習慣了吧？不習慣進入角色心裡。但你得習慣，否則你做不出讓人心動的劇情。」");
	d11->lines.push_back("沈奕恆：「簡單來說，第三人稱（他/她）比較適合敘述劇情、觀察角色。而第一人稱（我）能讓玩家更直接帶入角色的情緒與思考。沉浸式戀愛遊戲常使用第一人稱強化『我正在經歷這段戀情』的感覺。」");

	std::shared_ptr<Quiz> q11 = std::make_shared<Quiz>();
	q11->question = "沈奕恆：「Q1. 你正在寫一段角色告白的台詞，哪一句最容易讓玩家產生共鳴？」";
	q11->options.push_back("A. 他看著她，眼神中藏著情緒的風暴。");
	q11->options.push_back("B. 我看著她，眼神藏不住我胸口洶湧的情緒。");
	q11->options.push_back("C. 看著她，情緒有點複雜。");
	q11->options.push_back("D. 她低頭，他看著她沉默。");
	q11->v_score = {5, 10, 0, 5}; 
	q11->ansIndex = 1;
    q11->feedback.resize(4);
    q11->feedback[0] = "沈奕恆：「第三人稱，旁觀感較強。」";
    q11->feedback[1] = "沈奕恆（點頭）：「用『我』，讓玩家沒得逃。」";
    q11->feedback[2] = "沈奕恆：「過於簡略，情感不夠強烈。」";
    q11->feedback[3] = "沈奕恆：「純粹的動作描述，缺乏內心戲。」";

	std::shared_ptr<Quiz> q12 = std::make_shared<Quiz>();
	q12->question = "沈奕恆：「Q2. 老師說：『視角設計不是技術問題，是情感問題。』這句話的意思是？」";
	q12->options.push_back("A. 遊戲應該多用鏡頭特效");
	q12->options.push_back("B. 玩家要能從角色立場感受愛情");
	q12->options.push_back("C. 劇情要全用旁白描述才合理");
	q12->options.push_back("D. 玩家應該只看劇情，不做選擇");
	q12->v_score = {5, 10, 5, 0}; 
	q12->ansIndex = 1;
    q12->feedback.resize(4);
    q12->feedback[0] = "沈奕恆：「特效是輔助，核心在於情感傳達。」";
    q12->feedback[1] = "沈奕恆：「正確。視角是引導玩家共情的手段。\n主角（murmur）：「原來，不只是寫出來，而是要讓人心裡也動起來……」」";
    q12->feedback[2] = "沈奕恆：「旁白過多會削弱代入感。」";
    q12->feedback[3] = "沈奕恆：「選擇是互動的核心，能加強情感連結。」";

	std::shared_ptr<Dialog> d21 = std::make_shared<Dialog>();
	d21->lines.push_back("沈奕恆：「下一個主題：情緒迴圈與內隱選擇設計。讓選擇影響情緒，而不只是劇情走向。」");
	d21->lines.push_back("（下課後，教室只剩你和沈奕恆。他靠著窗邊，手裡拿著飲料吸了一口，然後問了一句：）");
	d21->lines.push_back("沈奕恆：「你喜歡那種選擇題，選 A 就戀愛成功、選 B 就失戀的遊戲嗎？」");
	d21->lines.push_back("你：「那太機械了，沒什麼感覺。」");
	d21->lines.push_back("沈奕恆（露出一抹幾不可見的微笑）：「我也是。真正好的選項……不該告訴你結果，而是讓你去『感覺』角色當下會怎麼想。」");
	d21->lines.push_back("（他走向講台，打開投影機。畫面顯示一個選擇分支圖，每個選項都標示著不同的角色情緒：「尷尬」「愧疚」「微妙喜歡」「不確定」。）");
	d21->lines.push_back("沈奕恆：「這叫『情緒迴圈』，不是給你看到結局的選項，而是讓你在心裡自己走到那個情緒裡。」");
	d21->lines.push_back("你：「所以……我們不是選結局，而是選情緒？」");
	d21->lines.push_back("沈奕恆：「對。感情不是一瞬間發生的，是在一次次細小選擇中，被引導出來的。」");
	d21->lines.push_back("沈奕恆：「所謂『內隱選擇設計』，就是選項表面看起來模糊，但其實暗藏情緒走向，引導玩家『體會』而非『知道』。角色的情緒反應應該連續地影響下一個選擇，而不是重設。」");
    d21->lines.push_back("沈奕恆：「例如，當你問『收到她的訊息，你最自然的反應是？』選項可能是：A. 秒回（可能導致焦慮）；B. 先假裝冷靜（可能導致壓抑）；C. 等她問第二次（可能導致防衛）。這些選項不一定有對錯，但會形塑角色走向哪種情感狀態。」");

	std::shared_ptr<Quiz> q21 = std::make_shared<Quiz>();
	q21->question = "沈奕恆：「Q1. 你要設計一個讓玩家感受到「被忽略」的戀愛選項，哪一個最有內隱情緒影響力？」";
	q21->options.push_back("A. 不讀訊息");
	q21->options.push_back("B. 傳訊息說「晚點再說」");
	q21->options.push_back("C. 點開對方限動不回訊息");
	q21->options.push_back("D. 跟對方說「先忙」但其實沒事做");
	q21->v_score = {0, 5, 10, 5}; 
	q21->ansIndex = 2;
    q21->feedback.resize(4);
    q21->feedback[0] = "沈奕恆：「直接不讀，對方可能只是認為你沒看到。」";
    q21->feedback[1] = "沈奕恆：「明確告知晚點回，至少有個交代。」";
    q21->feedback[2] = "沈奕恆：「是的。這種『已讀不回』式的行為，最能引發被忽略的猜測與不安。\n沈奕恆：「這不是最直接的，但會讓人一直想『他是不是故意的』。這種模糊，才最傷人。」」";
    q21->feedback[3] = "沈奕恆：「雖然是欺騙，但表面上還是給了理由。」";

	std::shared_ptr<Quiz> q22 = std::make_shared<Quiz>();
	q22->question = "沈奕恆：「Q2. 下列哪句敘事最能設計出讓玩家自己體會「遲疑中的心動」？」";
	q22->options.push_back("A. 我告訴她我喜歡她了。");
	q22->options.push_back("B. 我本來想傳訊息，結果停在打字框好幾分鐘。");
	q22->options.push_back("C. 我立刻按下送出鍵。");
	q22->options.push_back("D. 她走過來，我轉身走開。");
	q22->v_score = {5, 10, 5, 0}; 
	q22->ansIndex = 1;
    q22->feedback.resize(4);
    q22->feedback[0] = "沈奕恆：「這是結果，不是過程中的遲疑。」";
    q22->feedback[1] = "沈奕恆：「正確。行動前的猶豫，最能體現內心的波動。\n主角（murmur）：「原來一個卡住的瞬間，也能讓人心臟砰砰跳。」」";
    q22->feedback[2] = "沈奕恆：「太果斷了，沒有遲疑的空間。」";
    q22->feedback[3] = "沈奕恆：「這是逃避，不是心動的遲疑。」";

	std::shared_ptr<Dialog> d31 = std::make_shared<Dialog>();
	d31->lines.push_back("沈奕恆：「接著是多重視角與心理張力設計。目標是讓玩家同時理解『角色在想什麼』與『玩家自己在感受什麼』。」");
	d31->lines.push_back("（你和沈奕恆正在進行一項期末練習——用兩種視角寫一段「失約」的劇情：一個是主角被放鴿子的視角，另一個是放鴿子的那方視角。）");
	d31->lines.push_back("（沈奕恆坐在你旁邊，低著頭打字，一言不發。你忍不住偷瞄他的螢幕，上面寫著：）");
	d31->lines.push_back("（螢幕文字：『我明明也想去見他，但我真的不敢。我怕見了他，連保持距離這件事都做不到了。』）");
	d31->lines.push_back("（你心裡一震，剛想開口，他卻突然闔上筆電。）");
	d31->lines.push_back("沈奕恆（語氣平靜）：「多重視角可以讓情感更厚實，但要小心使用。太快揭露，情緒會提早釋放完；太慢揭露，玩家會抽離。」");
	d31->lines.push_back("你：「那你怎麼拿捏？」");
	d31->lines.push_back("沈奕恆（望著窗外）：「靠張力。讓兩個視角的感覺互相矛盾、交錯，但又不完全對立。像一條看不到終點的拉鋸戰，才讓人上癮。」");
	d31->lines.push_back("沈奕恆：「多重視角敘事，就是同時給出『主角視角』與『他人視角』，但資訊不對等，以此營造心理緊繃。心理張力不是用外在衝突製造高潮，而是用『情感的未說出口』與『理解落差』創造壓抑與張力。」");
    d31->lines.push_back("沈奕恆：「例如，玩家知道『某角色其實很在意主角』，但主角卻誤會他冷漠。此時玩家面臨的選擇不是『衝出去表白』，而是『是否忍住、等待』。這類選擇能夠累積心理張力，為後續情感爆發打底。」");

	std::shared_ptr<Quiz> q31 = std::make_shared<Quiz>();
	q31->question = "沈奕恆：「Q1. 你希望讓玩家在遊戲中同時感受到「他不來」與「他其實很在意」的矛盾效果，應該怎麼設計？」";
	q31->options.push_back("A. 他傳訊息說「最近很忙」");
	q31->options.push_back("B. 他沒來，但桌上有一杯還溫熱的咖啡");
	q31->options.push_back("C. 他直接打來說「別等我」");
	q31->options.push_back("D. 他在訊息中打了一大串解釋");
	q31->v_score = {0, 10, 5, 5}; 
	q31->ansIndex = 1;
    q31->feedback.resize(4);
    q31->feedback[0] = "沈奕恆：「這是常見的藉口，但缺乏『在意』的暗示。」";
    q31->feedback[1] = "沈奕恆：「是的。物品的溫度暗示了他不久前還在，營造了『在意但離開』的矛盾感。\n沈奕恆（低聲）：「溫度留下了他曾經在的證據……比千言萬語更難忘。」」";
    q31->feedback[2] = "沈奕恆：「太直接了，沒有留下懸念和矛盾空間。」";
    q31->feedback[3] = "沈奕恆：「解釋過多反而可能降低神秘感和張力。」";

	std::shared_ptr<Quiz> q32 = std::make_shared<Quiz>();
	q32->question = "沈奕恆：「Q2. 你設計了一段兩人吵架的劇情，想讓玩家明白「沈奕恆其實在壓抑情緒」但表面冷靜，應該怎麼寫他的台詞？」";
	q32->options.push_back("A. 「我沒事，你做什麼都可以。」");
	q32->options.push_back("B. 「我說了，這件事不重要。」");
	q32->options.push_back("C. 「……這樣也好，反正我們本來就不該太親近。」");
	q32->options.push_back("D. 「你想怎樣就怎樣。」");
	q32->v_score = {0, 5, 10, 5}; 
	q32->ansIndex = 2;
    q32->feedback.resize(4);
    q32->feedback[0] = "沈奕恆：「這句話太過順從，不像壓抑，更像放棄。」";
    q32->feedback[1] = "沈奕恆：「試圖轉移話題，但『壓抑』的感覺不夠強。」";
    q32->feedback[2] = "沈奕恆：「正確。這句話表面看似接受，實則充滿了未說出口的疏離和無奈，體現了壓抑。\n主角（murmur）：「好像真的沒什麼，但哪裡……讓人心裡很悶。」」";
    q32->feedback[3] = "沈奕恆：「帶有賭氣的成分，但壓抑的層次感不足。」";

	std::shared_ptr<Dialog> d41 = std::make_shared<Dialog>();
	d41->lines.push_back("沈奕恆：「討論動態對話系統與角色記憶反應。思考過去的選擇如何影響角色回應。」");
	d41->lines.push_back("（你這幾天跟沈奕恆的對話頻率越來越高。雖然他還是話少，但你總覺得，他好像記得你說過的每一句話。）");
	d41->lines.push_back("（今天在練習互動模擬，你故意輸入一句看似隨機的選項：）");
	d41->lines.push_back("你：「那你會記得我說過的話嗎？」");
	d41->lines.push_back("（沈奕恆愣了一下，然後淡淡地回：）");
	d41->lines.push_back("沈奕恆：「你不是說過你喜歡冷色調的封面設計嗎？我以為你也會比較喜歡這種回應方式。」");
	d41->lines.push_back("（你一時說不出話來。原來，他真的都有記住。）");
	d41->lines.push_back("（老師經過，看見你們的設計稿，點點頭。）");
	d41->lines.push_back("老師：「動態對話不是單純的『選項回應』，而是設計一種『有記憶的角色反應』——你今天對他怎麼說，他明天就會怎麼回答你。」");
	d41->lines.push_back("（沈奕恆看著螢幕，輕聲補一句：）");
	d41->lines.push_back("沈奕恆：「就像……你上次說過你害怕冷場，所以我才會現在主動說話。」");
	d41->lines.push_back("（你忽然覺得胸口有點悶——明明只是程式設計課，為什麼感覺像是在談心？）");
	d41->lines.push_back("沈奕恆：「動態對話系統，就是設計角色會記住玩家選項，並在後續互動中做出相應反應。玩家的行為會影響角色的信任度、態度改變，甚至劇情走向。這能讓角色慢慢記錄下玩家的選擇，使後續的情感爆發更有說服力。」");
    d41->lines.push_back("沈奕恆：「例如，如果玩家曾選擇忽略我提到的壓力，之後我在分組報告時可能會選擇和別人合作。但如果玩家曾主動詢問我的壓力，我之後可能會主動私訊說：『這次報告……我想跟你一組。』」");

	std::shared_ptr<Quiz> q41 = std::make_shared<Quiz>();
	q41->question = "沈奕恆：「Q1. 你希望讓我根據玩家過去是否「主動關心」來決定是否講真話，哪種設計方式較好？」";
	q41->options.push_back("A. 設定機率：關心過→30%會講真話");
	q41->options.push_back("B. 分兩種劇情線：關心過→進入我的回憶事件");
	q41->options.push_back("C. 讓玩家選項固定，劇情照常發展");
	q41->options.push_back("D. 加入我說謊的選項，增加趣味性");
	q41->v_score = {5, 10, 0, 5}; 
	q41->ansIndex = 1;
    q41->feedback.resize(4);
    q41->feedback[0] = "沈奕恆：「機率太隨機，無法體現玩家選擇的重要性。」";
    q41->feedback[1] = "沈奕恆：「是的。明確的劇情分支能讓玩家感受到選擇的影響力。\n老師：「好設計不靠運氣，而是讓選擇變得值得。」」";
    q41->feedback[2] = "沈奕恆：「這樣玩家的選擇就失去意義了。」";
    q41->feedback[3] = "沈奕恆：「說謊可以是一種反應，但核心是如何體現『記憶』。」";

	std::shared_ptr<Quiz> q42 = std::make_shared<Quiz>();
	q42->question = "沈奕恆：「Q2. 你設計了一段對話，想讓玩家從我的反應中感受到我記得過去的互動，哪句台詞最適合？」";
	q42->options.push_back("A. 「……沒什麼，就照流程走。」");
	q42->options.push_back("B. 「你那時不是說這樣會讓人沒安全感嗎？」");
	q42->options.push_back("C. 「嗯，我記不得了。」");
	q42->options.push_back("D. 「每次都這樣，也挺正常的。」");
	q42->v_score = {0, 10, 5, 5}; 
	q42->ansIndex = 1;
    q42->feedback.resize(4);
    q42->feedback[0] = "沈奕恆：「這句話聽起來很疏離，不像記得。」";
    q42->feedback[1] = "沈奕恆：「正確。引用過去的對話，直接體現了記憶。\n主角（murmur）：「他說得很輕……但我記得我講過這句話是在……我們第一次吵架之後。」」";
    q42->feedback[2] = "沈奕恆：「直接否認，與目的相反。」";
    q42->feedback[3] = "沈奕恆：「這句話比較消極，沒有展現對特定互動的記憶。」";

	std::shared_ptr<Dialog> d51 = std::make_shared<Dialog>();
	d51->lines.push_back("（這段教學讓「情感記憶」逐漸浮現：沈奕恆雖然不主動，但一點一滴的累積，讓情緒的壓抑變得更真實、更有力。）");
	d51->lines.push_back("沈奕恆：「最後一個模組：情緒崩潰點與玩家代入的情感爆發。目標是了解如何設計情感崩潰點，使玩家能夠深刻體會角色內心的掙扎與解放。」");
	d51->lines.push_back("（這幾天來，你和沈奕恆之間的對話越來越少，氣氛也逐漸變得有些緊張。你注意到，他的眼神變得更冷淡，甚至對你的問題也不再像以前那樣細心回答。）");
	d51->lines.push_back("（這一切似乎是無形中積累的結果，無論是小小的冷場，還是那個不經意的回應，漸漸地他似乎在逃避你。）");
	d51->lines.push_back("（今天你決定找沈奕恆，談一談這段時間的變化。你知道，這場對話可能會決定你們之間的未來。）");
	d51->lines.push_back("你：「這幾天，我注意到你的變化。是不是在刻意疏遠我？」"); 
	d51->lines.push_back("（沈奕恆心頭一緊，低頭不語。這一刻，他終於體會到你心中的掙扎，原來你也在忍耐。）"); 
	d51->lines.push_back("沈奕恆：「其實，我在害怕。如果我一直靠近你，會不會讓你覺得負擔？你之前說過你討厭情感依賴，我擔心……會讓你更遠離我。」"); 
	d51->lines.push_back("（你愣住，眼中閃過一絲迷茫。隨後，你的表情變得更加複雜。）"); 
	d51->lines.push_back("你：「你以為……我一直保持距離，是因為你依賴我嗎？不，我是因為……我自己不敢再靠近你。」"); 
	d51->lines.push_back("（沈奕恆感到一陣震驚，這句話像是敲響了他內心的鐘聲。他沒想到，你一直在壓抑自己的情感，將內心的柔軟部分隱藏在冷靜的外表下。）"); 
	d51->lines.push_back("你（眼神變得堅定）：「這段時間，我一直在想我們之間的關係。每當我接近你，心裡總會有一種莫名的恐懼。那是……我不敢面對的情感。」"); 
	d51->lines.push_back("（沈奕恆感受到一種強烈的情感波動，這不僅僅是角色之間的對話，更像是心靈深處的碰撞。）");
	d51->lines.push_back("你：「我一直以為，控制情感是最安全的方式，但現在我知道，我不能再繼續這樣逃避下去。」"); 
    d51->lines.push_back("沈奕恆：「情感爆發點的設計，是情感積壓後的釋放，需注意時機與玩家代入感。我的冷靜與矛盾，其實是一種內心的自我防衛。當情感爆發時，玩家與角色的情感連結會更加緊密。」");

	std::shared_ptr<Quiz> q51 = std::make_shared<Quiz>();
	q51->question = "沈奕恆：「Q1. 當設計情感崩潰點時，以下哪個元素最能增強情感的爆發力？」";
	q51->options.push_back("A. 強烈的衝突與對抗");
	q51->options.push_back("B. 輕描淡寫的反應，讓情感逐漸浮現，最後爆發");
	q51->options.push_back("C. 一個突然的、戲劇性的事件");
	q51->options.push_back("D. 玩家無法選擇的情節，讓角色的情感主導一切");
	q51->v_score = {5, 10, 5, 0}; 
	q51->ansIndex = 1;
    q51->feedback.resize(4);
    q51->feedback[0] = "沈奕恆：「直接衝突可能有效，但細膩的鋪陳後爆發，張力更強。」";
    q51->feedback[1] = "沈奕恆：「正確。情感的爆發應是逐步累積的，這樣才有足夠的衝擊力。\n老師：「情感的爆發應該是逐步累積的，這樣才有足夠的衝擊力。」」"; 
    q51->feedback[2] = "沈奕恆：「突然事件可以觸發，但情感基礎的鋪墊更為重要。」";
    q51->feedback[3] = "沈奕恆：「玩家的選擇和代入感很重要，完全被動可能削弱體驗。」";

	std::shared_ptr<Quiz> q52 = std::make_shared<Quiz>();
	q52->question = "沈奕恆：「Q2. 若想讓我的情感崩潰更具震撼感，哪種設計最能提升效果？」";
	q52->options.push_back("A. 讓玩家選擇是否解開我的內心");
	q52->options.push_back("B. 讓我主動揭示自己的情感過程，帶有回憶的情感描寫");
	q52->options.push_back("C. 讓我保持冷漠，直到最終揭示內心");
	q52->options.push_back("D. 讓我在玩家的選擇中始終保持淡定");
	q52->v_score = {5, 10, 5, 0}; 
	q52->ansIndex = 1;
    q52->feedback.resize(4);
    q52->feedback[0] = "沈奕恆：「玩家的選擇很重要，但內心的揭示方式也需考量。」";
    q52->feedback[1] = "沈奕恆：「是的。由角色主動、細膩地展現內心轉折，能讓玩家更深地共情。\n主角（murmur）：「這段時間我一直以為他冷漠，沒想到……是他在掙扎、在逃避。」」";
    q52->feedback[2] = "沈奕恆：「一直冷漠到最後才揭示，可能鋪陳不足，爆發力不夠。」";
    q52->feedback[3] = "沈奕恆：「如果角色始終淡定，就沒有所謂的情感崩潰了。」";

	std::shared_ptr<Dialog> e1 = std::make_shared<Dialog>(); // Good end for initC
	e1->type = DialogType::GOODEND;
	e1->lines.push_back("（你們終於突破了那層無形的障礙，沈奕恆的情感終於被釋放，他不再壓抑自己的情感，兩人之間的關係終於有了突破。）");
	e1->lines.push_back("（你與沈奕恆站在教室窗前，看著外面漸漸暗下來的天空。你們的眼神交會，彼此之間不再有疏離感，只有無言的默契。）");
	e1->lines.push_back("沈奕恆（輕聲）：「或許，我們不需要再理智到冷血。只要你能在我身邊，我就夠了。」");
	e1->lines.push_back("（你們的手指微微碰觸，彼此都感受到對方內心的那份溫暖。）");
	e1->lines.push_back("（畫面漸暗，顯示文字）");
	e1->lines.push_back("「你成功通關了《遊戲程式與戀愛學特訓班》：沈奕恆路線｜攻略達成」");

	std::shared_ptr<Dialog> e2 = std::make_shared<Dialog>(); // Bad end for initC
	e2->type = DialogType::BADEND;
	e2->lines.push_back("（沈奕恆站在你面前，沉默片刻。你能感受到他內心的掙扎，卻依然無法觸及到他的內心。）");
	e2->lines.push_back("沈奕恆（低頭）：「或許，我不應該再繼續這樣逃避。我不擅長表達情感，但這不意味著我不在乎。」");
	e2->lines.push_back("（你聽見他輕聲自語，心中有一種說不清的遺憾。也許，他的心門永遠無法打開，或許，你還需要更長時間來解開他心中的結。）");
	e2->lines.push_back("（畫面轉黑，顯示文字）");
	e2->lines.push_back("「你未能通關《遊戲程式與戀愛學特訓班》：沈奕恆路線｜未攻略成功，但故事還沒結束……？」");

	NPC& character_npc = DialogSystem::getInstance().addNPC(go, {
        d11, q11, q12, d21, q21, q22, d31, q31, q32, 
        d41, q41, q42, d51, q51, q52, e1, e2
    });
	if (character_npc.go) character_npc.go->invMass = 0;
	character_npc.inDialog = false; 
	character_npc.routeEnabled = true; 
}