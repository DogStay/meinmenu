// ЕДИНСТВЕННАЯ ТОЧКА СОПРИКОСНОВЕНИЯ С ВАНИЛЬНЫМИ КЛАССАМИ.
//
// Всё остальное в моде — самодостаточные вьюхи (TFM_LoadingView,
// TFM_QueueView, TFM_TimerView), которые ничего не знают про DayZ-классы
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
    protected ref TFM_LoadingView m_TFMView;
    protected float m_TFMLastTick;

    override void Show()
    {
        super.Show();

        if (!m_TFMView)
            m_TFMView = new TFM_LoadingView();

        // Ванильные виджеты не трогаем: наш layout создан позже и лежит
        // поверх, полностью перекрывая экран непрозрачным фоном.
        m_TFMView.Show(true);
    }

    override void Hide(bool force = false)
    {
        super.Hide(force);

        if (m_TFMView)
        {
            m_TFMView.Destroy();
            m_TFMView = null;
        }
    }

    //! Прогресс приходит из ванильного кода загрузки; 0..1.
    void TFMSetProgress(float progress)
    {
        if (m_TFMView)
            m_TFMView.SetProgress(progress);
    }

    void TFMUpdate(float timeslice)
    {
        if (m_TFMView)
            m_TFMView.Update(timeslice);
    }

}

// ---------------------------------------------------------------------------
// 05 SERVER QUEUE
// ---------------------------------------------------------------------------

modded class LoginQueueBase
{
    protected ref TFM_QueueView m_TFMQueue;

    override Widget Init()
    {
        Widget root = super.Init();

        if (root)
            TFMHideChildren(root);

        m_TFMQueue = new TFM_QueueView();

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFMQueue)
            m_TFMQueue.Update(timeslice);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_TFMQueue)
        {
            if (m_TFMQueue.OnClick(w))
                return true;

            if (m_TFMQueue.GetCancelWidget() == w)
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
        if (m_TFMQueue && m_TFMQueue.OnMouseEnter(w))
            return true;

        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (m_TFMQueue && m_TFMQueue.OnMouseLeave(w))
            return true;

        return super.OnMouseLeave(w, enterW, x, y);
    }

    //! Вызывается ванильным кодом при обновлении позиции в очереди.
    void TFMSetPosition(int position, int total)
    {
        if (m_TFMQueue)
            m_TFMQueue.SetPosition(position, total);
    }

    protected void TFMHideChildren(Widget root)
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
    protected ref TFM_TimerView m_TFMTimer;

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

        m_TFMTimer = new TFM_TimerView(TFM_TimerMode.LOGIN);

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFMTimer)
            m_TFMTimer.Update(timeslice);
    }

    void TFMSetTime(float seconds, float total)
    {
        if (m_TFMTimer)
            m_TFMTimer.SetTime(seconds, total);
    }
}

// ---------------------------------------------------------------------------
// 07 RESPAWN TIMER
// ---------------------------------------------------------------------------

modded class RespawnDialogue
{
    protected ref TFM_TimerView m_TFMRespawn;

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

        m_TFMRespawn = new TFM_TimerView(TFM_TimerMode.RESPAWN);

        return root;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_TFMRespawn)
            m_TFMRespawn.Update(timeslice);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_TFMRespawn && m_TFMRespawn.GetRespawnWidget() == w)
        {
            if (!m_TFMRespawn.IsRespawnEnabled())
                return true;    //!< DISABLED до 00:00

            // ВАЖНО: сюда нужно подставить вызов ванильного респавна из
            // RespawnDialogue вашего билда — наш ButtonWidget не тот, на
            // который завязан оригинальный обработчик. См. README.
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    void TFMSetTime(float seconds, float total)
    {
        if (m_TFMRespawn)
            m_TFMRespawn.SetTime(seconds, total);
    }
}
