nginx源码导读

1、main 函数逻辑

2、master 进程的循环 ngx_master_process_cycle
	a、启动工作进程 ngx_start_worker_processes
	b、启动缓存管理进程 ngx_start_cache_manager_processes
	c、死循环
	for(){
		// 如果退出，通知工作进程
		if (ngx_terminate) ngx_signal_worker_processes(SIGKILL);
		
		// 如果退出，通知工作进程
		if (ngx_quit) ngx_signal_worker_processes(NGX_SHUTDOWN_SIGNAL)
		
		// 如果配置改变，通知工作进程
	}
	
3、worker 进程的循环，核心在 ngx_process_events_and_timers
    for ( ;; ) {
	// 处理事件和定时器
        ngx_process_events_and_timers(cycle);

	// 如果退出，调用各模块的退出函数
        if (ngx_terminate || ngx_quit) {
            for (i = 0; cycle->modules[i]; i++) {
                if (cycle->modules[i]->exit_process) {
                    cycle->modules[i]->exit_process(cycle);
                }
            }
            ngx_master_process_exit(cycle);
        }
	// 如果重新配置
        if (ngx_reconfigure) {
            ngx_reconfigure = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reconfiguring");

            cycle = ngx_init_cycle(cycle);
            if (cycle == NULL) {
                cycle = (ngx_cycle_t *) ngx_cycle;
                continue;
            }

            ngx_cycle = cycle;
        }
		// 如果重新打开
        if (ngx_reopen) {
            ngx_reopen = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
            ngx_reopen_files(cycle, (ngx_uid_t) -1);
        }
    }
