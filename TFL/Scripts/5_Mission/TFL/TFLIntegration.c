// ЕДИНСТВЕННАЯ ТОЧКА СОПРИКОСНОВЕНИЯ С ВАНИЛЬНЫМИ КЛАССАМИ.
//
// Всё остальное в моде — самодостаточные вьюхи (TFL_LoadingView,
// TFL_QueueView, TFL_TimerView), которые ничего не знают про DayZ-классы
// и могут быть подняты откуда угодно.
//
// Имена и сигнатуры ванильных классов загрузки/очереди/таймеров меняются
// между билдами DayZ. Если компилятор ругается на этот файл — правьте
// (или удаляйте) только его: остальной мод от этого не ломается,
// главное меню продолжит работать.
//
// Сверять по скриптам своей версии игры:
//   scripts/5_Mission/gui/LoadingScreen.c
//   scripts/5_Mission/gui/LoginQueueBase.c
//   scripts/5_Mission/gui/LoginTimeBase.c
//   scripts/5_Mission/gui/RespawnDialogue.c

// ---------------------------------------------------------------------------
// 03/04 LOADING SCREEN
// ---------------------------------------------------------------------------

modded class LoadingScreen
{
    protected ref TFL_LoadingView m_TFLView;
    protected float m_TFLLastTick;

    override void Show()
    {
        super.Show();

        if (!m_TFLView)
            m_TFLView = new TFL_LoadingView();

        // Ванильные виджеты не трогаем: наш layout создан позже и лежит
        // поверх, полностью перекрывая экран непрозрачным фоном.
        m_TFLView.Show(true);
    }

    override void Hide(bool force = false)
    {
        super.Hide(force);

        if (m_TFLView)
        {
            m_TFLView.Destroy();
            m_TFLView = null;
        }
    }

    //! Прогресс приходит из ванильного кода загрузки; 0..1.
    void TFLSetProgress(float progress)
    {
        if (m_TFLView)
            m_TFLView.SetProgress(progress);
    }

    void TFLUpdate(float timeslice)
    {
        if (m_TFLView)
            m_TFLView.Update(timeslice);
    }

}

// ---------------------------------------------------------------------------
// 05 SERVER QUEUE
// ---------------------------------------------------------------------------

modded class LoginQueueBase
{
    protected ref TFL_QueueView m_TFLQueue;

    override Widget Init()
    {
        Widget root = super.Init();

        if (root)
            TFLHideChildren(root);

        m_TFLQueue = new TFL_QueueView();

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFLQueue)
            m_TFLQueue.Update(timeslice);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_TFLQueue)
        {
            if (m_TFLQueue.OnClick(w))
                return true;

            if (m_TFLQueue.GetCancelWidget() == w)
            {
                // Отмена очереди = обычный выход из подключения.
                GetGame().GetUIManager().Back();
                return true;
            }
        }

        return super.OnClick(w, x, y, button);
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (m_TFLQueue && m_TFLQueue.OnMouseEnter(w))
            return true;

        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (m_TFLQueue && m_TFLQueue.OnMouseLeave(w))
            return true;

        return super.OnMouseLeave(w, enterW, x, y);
    }

    //! Вызывается ванильным кодом при обновлении позиции в очереди.
    void TFLSetPosition(int position, int total)
    {
        if (m_TFLQueue)
            m_TFLQueue.SetPosition(position, total);
    }

    protected void TFLHideChildren(Widget root)
    {
        Widget child = root.GetChildren();

        while (child)
        {
            child.Show(false);
            child = child.GetSibling();
        }
    }
}

// ---------------------------------------------------------------------------
// 06 LOGIN TIMER
// ---------------------------------------------------------------------------

modded class LoginTimeBase
{
    protected ref TFL_TimerView m_TFLTimer;

    override Widget Init()
    {
        Widget root = super.Init();

        if (root)
        {
            Widget child = root.GetChildren();

            while (child)
            {
                child.Show(false);
                child = child.GetSibling();
            }
        }

        m_TFLTimer = new TFL_TimerView(TFL_TimerMode.LOGIN);

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFLTimer)
            m_TFLTimer.Update(timeslice);
    }

    void TFLSetTime(float seconds, float total)
    {
        if (m_TFLTimer)
            m_TFLTimer.SetTime(seconds, total);
    }
}

// ---------------------------------------------------------------------------
// 07 RESPAWN TIMER
// ---------------------------------------------------------------------------

modded class RespawnDialogue
{
    protected ref TFL_TimerView m_TFLRespawn;

    override Widget Init()
    {
        Widget root = super.Init();

        if (root)
        {
            Widget child = root.GetChildren();

            while (child)
            {
                child.Show(false);
                child = child.GetSibling();
            }
        }

        m_TFLRespawn = new TFL_TimerView(TFL_TimerMode.RESPAWN);

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFLRespawn)
            m_TFLRespawn.Update(timeslice);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_TFLRespawn && m_TFLRespawn.GetRespawnWidget() == w)
        {
            if (!m_TFLRespawn.IsRespawnEnabled())
                return true;    //!< DISABLED до 00:00

            // ВАЖНО: сюда нужно подставить вызов ванильного респавна из
            // RespawnDialogue вашего билда — наш ButtonWidget не тот, на
            // который завязан оригинальный обработчик. См. README.
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void TFLSetTime(float seconds, float total)
    {
        if (m_TFLRespawn)
            m_TFLRespawn.SetTime(seconds, total);
    }
}
