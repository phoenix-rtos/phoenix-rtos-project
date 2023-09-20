define _ps
	if $arg1 == -1 || $arg0->tid == $arg1
		if threads->current[0] == $arg0
			printf "%3d* ", $arg0->tid
		else
			printf "%3d  ", $arg0->tid
		end
		out $arg0->state
		printf "\t%p  %p  ", $arg0, $arg0->kstack
		out $arg0->start
		printf "\t%p:  ", $arg0->ctx.ret
		info symbol $arg0->ctx.ret
		if $arg0->process != 0 && $arg0->process != -1
			printf "\t(process_t *)%p %s pid: %d pmap: ", $arg0->process, $arg0->process->name, $arg0->process->pid
			out $arg0->process->pmap
			echo \n
		end
	end

	if $arg0->tid == $tid
		lsctx $arg0->ctx

		set $_sp=$sp
		set $_lr=$lr
		set $_fp=$r11
		set $_pc=$pc
		set $_r0=$r0
		set $_r1=$r1
		set $_r2=$r2
		set $_r3=$r3
		set $_r4=$r4
		set $sp=$arg0->ctx->sp
		set $lr=$arg0->ctx->lr
		set $r11=$arg0->ctx->fp
		set $pc=$arg0->ctx->ret
		set $r0=$arg0->ctx->reg[0]
		set $r1=$arg0->ctx->reg[1]
		set $r2=$arg0->ctx->reg[2]
		set $r3=$arg0->ctx->reg[3]
		set $r4=$arg0->ctx->reg[4]
		backtrace
		set $sp=$_sp
		set $lr=$_lr
		set $r11=$_fp
		set $pc=$_pc
		set $r0=$_r0
		set $r1=$_r1
		set $r2=$_r2
		set $r3=$_r3
		set $r4=$_r4
	end
end
define __ps
    set $t = $arg0
    out $t->state
    printf " "
    #printf "%3d\t", $t->id
    printf "(thread_t *)%p ", $t
    if $t->context != 0
        printf "(cpu_context_t *)%p pc: %p ", $t->context, $t->context.pc
        info symbol $t->context.pc
    else
        printf "(cpu_context_t *)%p", $t->context
    end
    #out $arg0->state
    if $t->process != 0 && $t->process != -1
        printf "\t(process_t *)%p [%s] ", $t->process, $t->process->path
        set $arg = $t->process->argv
        while $arg != 0 && *$arg != 0
            printf "%s ", *$arg
            set $arg = $arg + 1
        end
        #out $arg0->process->state
        set $uctx = (cpu_context_t*)(((thread_t *)$arg0)->kstack + ((thread_t *)$arg0)->kstacksz - sizeof(cpu_context_t))
        printf "\n\t userctx: (cpu_context_t *)%p", $uctx
    end
    echo \n
end

# for internal use
define offsetof
    set $rc = (unsigned char*)&(($arg0 *)0)->$arg1 - (char*)0
end

define rbtree_entry
    offsetof $arg1 $arg2
    set $rc = (($arg1 *) ((unsigned char*) ($arg0) - $rc))
end

define rb_get_first
    set $rc = (rbnode_t *) $arg0
    if $rc != 0
        while $rc->left != 0
            set $rc = $rc->left
        end
    end
end

define rb_get_next
    set $rc = (rbnode_t *) $arg0
    if $rc->right != 0
        rb_get_first ($rc->right)
    else
        while $rc->parent != 0 && $rc == $rc->parent->right
            set $rc = $rc->parent
        end
        set $rc = $rc->parent
    end
end

define thread_get_first
    rb_get_first threads_common.id.root
    set $rnode = $rc
	rbtree_entry $rnode thread_t idlinkage
end

define thread_get_next
    set $_t = (thread_t*) $arg0
    rb_get_next &$_t->idlinkage
    if $rc != 0
        set $rnode = $rc
        rbtree_entry $rnode thread_t idlinkage
    end
end

define ps
    thread_get_first
    set $thread = $rc
	if $argc > 0
		set $tid = $arg0
	else
		set $tid = -1
	end

	while $thread != 0
        __ps $thread $tid
        thread_get_next $thread
		set $thread = $rc
	end
end

document ps
Lists threads
Use "ps <Thread ID> to show register dump and call stack for given thread
end


define swapctx
    set $ctx=(cpu_context_t *)$arg0

    set $r0=$ctx->r0
    set $r1=$ctx->r1
    set $r2=$ctx->r2
    set $r3=$ctx->r3
    set $r4=$ctx->r4
    set $r5=$ctx->r5
    set $r6=$ctx->r6
    set $r7=$ctx->r7
    set $r8=$ctx->r8
    set $r9=$ctx->r9
    set $r10=$ctx->r10
    set $r11=$ctx->fp
    set $r12=$ctx->ip
    set $r13=$ctx->sp
    set $r14=$ctx->lr
    set $r15=$ctx->pc

    backtrace
end


document swapctx
Swaps registers (including pc and stack pointer) to the given context registers. Then prints backtrace.
end

define swapexctx
	set $ctx=(exc_context_t *)$arg0

    set $r0=$ctx->r0
    set $r1=$ctx->r1
    set $r2=$ctx->r2
    set $r3=$ctx->r3
    set $r4=$ctx->r4
    set $r5=$ctx->r5
    set $r6=$ctx->r6
    set $r7=$ctx->r7
    set $r8=$ctx->r8
    set $r9=$ctx->r9
    set $r10=$ctx->r10
    set $r11=$ctx->fp
    set $r12=$ctx->ip
    set $r13=$ctx->sp
    set $r14=$ctx->lr
    set $r15=$ctx->pc

    backtrace
end

define userctx
    print (cpu_context_t*)(((thread_t *)$arg0)->kstack + ((thread_t *)$arg0)->kstacksz - sizeof(cpu_context_t))
end

document userctx
Gets the userspace cpu_context_t from thread pointer
end



define lsctx
	set $ctx=(cpu_context_t *)$arg0

	set $i=0
	while $i < 5
		printf "  r%d=0x%08x  r%d=0x%08x  r%d=0x%08x\n", \
				$i, $ctx->reg[$i], $i+5, $ctx->reg[$i+5], $i+10, $ctx->reg[$i+10]
		set $i+=1
	end
	printf "  cpsr=0x%08x  pc=0x%08x\n", $ctx->psr, $ctx->ret

	printf "\t NZCVQITJ....<GE><-IT->EAIFT<-M->\n\t "
	set $i=32
	while $i > 0
		set $i = $i - 1
		if $ctx->psr & (1 << $i)
			printf "1"
		else
			printf "-"
		end
	end
	printf "\n"
end

document lsctx
Pretty print CPU context
end

define restart
	monitor reset
	set $pc=_start
	tbreak _stack_init
	c

	tbreak main
	c

end


define reload
	monitor reset
	load
	tbreak _stack_init
	c

	tbreak main
	c

end


define btctx
	set $ctx=(cpu_context_t *)$arg0

	set $_sp=$sp
	set $_lr=$lr
	set $_fp=$r11
	set $_pc=$pc
	set $sp=$ctx->sp
	set $lr=$ctx->lr
	set $r11=$ctx->fp
	set $pc=$ctx->pc
	backtrace
	set $sp=$_sp
	set $lr=$_lr
	set $r11=$_fp
	set $pc=$_pc
end

define btexctx
	set $ctx=(exc_context_t *)$arg0

	set $_sp=$sp
	set $_lr=$lr
	set $_fp=$r11
	set $_pc=$pc
	set $sp=$ctx->sp
	set $lr=$ctx->lr
	set $r11=$ctx->fp
	set $pc=$ctx->pc
	backtrace
	set $sp=$_sp
	set $lr=$_lr
	set $r11=$_fp
	set $pc=$_pc
end

define btthread
    set $thread=(thread_t*)$arg0
	set $ctx=$thread->context
    set $uctx=(cpu_context_t *)($thread->kstack + $thread->kstacksz - sizeof(cpu_context_t))

	set $_sp=$sp
	set $_lr=$lr
	set $_fp=$r11
	set $_pc=$pc
	set $sp=$ctx->sp
	set $lr=$ctx->lr
	set $r11=$ctx->fp
	set $pc=$ctx->pc
	backtrace

	set $sp=$uctx->sp
	set $lr=$uctx->lr
	set $r11=$uctx->fp
	set $pc=$uctx->pc
	backtrace

	set $sp=$_sp
	set $lr=$_lr
	set $r11=$_fp
	set $pc=$_pc
end


define asid_stat
	set $i=1
	while $i < 256
		printf "%02x: ", $i
		set $j = 1
		while $j < asid_map[$i].wear && $j < 80
			echo =
			set	$j += 1
		end
		if $j == 80
			printf "|\n"
		else
			printf "\n"
		end
		set	$i += 1
	end
	printf "1st hash hit: %d\n2nd hash hit: %d\nforced: %d\n", asid_hash1Hit, asid_hash2Hit, asid_forced
end


define ddr_ctrl_init
	set ((CCM_Type *) 0x4006B000)->CCGR4 |= 0x22a00000
	set ((CCM_Type *) 0x4006B000)->CCGR3 |= 0x00004022
	set ((CCM_Type *) 0x4006B000)->CCGR2 |= 0x0fee0000
	set ((SCSC_Type *)0x40052000)->SOSC_CTR |= 0x1
	set ((CCM_Type *) 0x4006B000)->CIMR  |= 0x40
	set ((CCM_Type *) 0x4006B000)->CCR   |= 0x1000
	set ((CCM_Type *) 0x4006B000)->CLPCR &= ~0x800

	while (((CCM_Type *)0x4006B000)->CSR & 0x20) == 0
	   p/x ((CCM_Type *)(0x4006B000))->CSR & 0x20
	end

	set ((ANADIG_Type *)0x40050000)->PLL1_CTRL = 0x00002003
	set ((ANADIG_Type *)0x40050000)->PLL1_PFD  = (((ANADIG_Type *)0x40050000u)->PLL1_PFD & ~0x3F3F0000) | (18 << 16) | (31 << 24)
	set ((ANADIG_Type *)0x40050000)->PLL3_CTRL = 0x00003040

	set ((CCM_Type *)0x4006B000)->CCGR4 |= 0x00C00000
	set ((CCM_Type *)0x4006B000)->CCSR   = 0xF003FF64
	set ((CCM_Type *)0x4006B000)->CACRR  = 0x00000809
	set ((ANADIG_Type *)0x40050000)->PLL2_CTRL = 0x00002002
	set ((ANADIG_Type *)0x40050000)->PLL5_CTRL = 0x00002001
	set ((CCM_Type *)0x4006B000)->CCGR6 |= 0x30000000

	set $reg = &(((IOMUXC_Type *)0x40048000)->DDR_RESETB)
	while $reg <= &(((IOMUXC_Type *)0x40048000)->DUMMY_DDRBYTE2)
		set *$reg = 0x00010140
		set $reg++
	end

	set ((IOMUXC_Type *)0x40048000)->DDR_CLK_0 |= 0x10000
	set ((IOMUXC_Type *)0x40048000)->DDR_DQS_0 |= 0x10000
	set ((IOMUXC_Type *)0x40048000)->DDR_DQS_1 |= 0x10000

	set ((DDRMC_Type *)0x400AE000)->CR00 = 0x00000600

	set ((DDRMC_Type *)0x400AE000)->CR02 = 0x00000020

	set ((DDRMC_Type *)0x400AE000)->CR10 = 0x00013880
	set ((DDRMC_Type *)0x400AE000)->CR11 = 0x00030D40
	set ((DDRMC_Type *)0x400AE000)->CR12 = 0x0000050c
	set ((DDRMC_Type *)0x400AE000)->CR13 = 0x15040400
	set ((DDRMC_Type *)0x400AE000)->CR14 = 0x1406040F
	set ((DDRMC_Type *)0x400AE000)->CR16 = 0x04040000
	set ((DDRMC_Type *)0x400AE000)->CR17 = 0x006DB00C
	set ((DDRMC_Type *)0x400AE000)->CR18 = 0x00000403

	set ((DDRMC_Type *)0x400AE000)->CR20 = 0x01000000
	set ((DDRMC_Type *)0x400AE000)->CR21 = 0x00060001
	set ((DDRMC_Type *)0x400AE000)->CR22 = 0x000C0000
	set ((DDRMC_Type *)0x400AE000)->CR23 = 0x03000200
	set ((DDRMC_Type *)0x400AE000)->CR24 = 0x00000006
	set ((DDRMC_Type *)0x400AE000)->CR25 = 0x00010000
	set ((DDRMC_Type *)0x400AE000)->CR26 = 0x0C30002C

	set ((DDRMC_Type *)0x400AE000)->CR28 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR29 = 0x00000003
	set ((DDRMC_Type *)0x400AE000)->CR30 = 0x0000000A
	set ((DDRMC_Type *)0x400AE000)->CR31 = 0x003001D4
	set ((DDRMC_Type *)0x400AE000)->CR33 = 0x00010000
	set ((DDRMC_Type *)0x400AE000)->CR34 = 0x00050500

	set ((DDRMC_Type *)0x400AE000)->CR38 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR39 = 0x04001002

	set ((DDRMC_Type *)0x400AE000)->CR41 = 0x00000001

	set ((DDRMC_Type *)0x400AE000)->CR45 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR46 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR47 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR48 = 0x00460420
	set ((DDRMC_Type *)0x400AE000)->CR49 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR51 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR57 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR66 = 0x01000200
	set ((DDRMC_Type *)0x400AE000)->CR67 = 0x00000040

	set ((DDRMC_Type *)0x400AE000)->CR69 = 0x00000200
	set ((DDRMC_Type *)0x400AE000)->CR70 = 0x00000040
	set ((DDRMC_Type *)0x400AE000)->CR71 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR72 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR73 = 0x0a010300
	set ((DDRMC_Type *)0x400AE000)->CR74 = 0x01014040
	set ((DDRMC_Type *)0x400AE000)->CR75 = 0x01010101
	set ((DDRMC_Type *)0x400AE000)->CR76 = 0x03030100
	set ((DDRMC_Type *)0x400AE000)->CR77 = 0x01000101
	set ((DDRMC_Type *)0x400AE000)->CR78 = 0x0700000C
	set ((DDRMC_Type *)0x400AE000)->CR79 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR82 = 0x10000000

	set ((DDRMC_Type *)0x400AE000)->CR87 = 0x01000000
	set ((DDRMC_Type *)0x400AE000)->CR88 = 0x00040000
	set ((DDRMC_Type *)0x400AE000)->CR89 = 0x00000002

	set ((DDRMC_Type *)0x400AE000)->CR91 = 0x00020000
	set ((DDRMC_Type *)0x400AE000)->CR92 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR96 = 0x00002819
	set ((DDRMC_Type *)0x400AE000)->CR97 = 0x01000000
	set ((DDRMC_Type *)0x400AE000)->CR98 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR99 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR102 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR105 = 0x00001800
	set ((DDRMC_Type *)0x400AE000)->CR106 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR110 = 0x00000018

	set ((DDRMC_Type *)0x400AE000)->CR114 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR115 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR117 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR118 = 0x01010000
	set ((DDRMC_Type *)0x400AE000)->CR119 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR120 = 0x02020000
	set ((DDRMC_Type *)0x400AE000)->CR121 = 0x00000202
	set ((DDRMC_Type *)0x400AE000)->CR122 = 0x01010064
	set ((DDRMC_Type *)0x400AE000)->CR123 = 0x00010101
	set ((DDRMC_Type *)0x400AE000)->CR124 = 0x00000064
	set ((DDRMC_Type *)0x400AE000)->CR125 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR126 = 0x00000800
	set ((DDRMC_Type *)0x400AE000)->CR127 = 0x00000000

	set ((DDRMC_Type *)0x400AE000)->CR131 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->CR132 = 0x00000506

	set ((DDRMC_Type *)0x400AE000)->CR137 = 0x00020000
	set ((DDRMC_Type *)0x400AE000)->CR138 = 0x01000100
	set ((DDRMC_Type *)0x400AE000)->CR139 = 0x04070303
	set ((DDRMC_Type *)0x400AE000)->CR140 = 0x00000040

	set ((DDRMC_Type *)0x400AE000)->CR143 = 0x06000080
	set ((DDRMC_Type *)0x400AE000)->CR144 = 0x04070303
	set ((DDRMC_Type *)0x400AE000)->CR145 = 0x00000040
	set ((DDRMC_Type *)0x400AE000)->CR146 = 0x00000040
	set ((DDRMC_Type *)0x400AE000)->CR147 = 0x000F0000
	set ((DDRMC_Type *)0x400AE000)->CR148 = 0x000F0000

	set ((DDRMC_Type *)0x400AE000)->CR151 = 0x00000101

	set ((DDRMC_Type *)0x400AE000)->CR154 = 0x682C4000
	set ((DDRMC_Type *)0x400AE000)->CR155 = 0x00000012

	set ((DDRMC_Type *)0x400AE000)->CR158 = 0x00000006

	set ((DDRMC_Type *)0x400AE000)->CR161 = 0x00010202

	set ((DDRMC_Type *)0x400AE000)->PHY00 = 0x00002613
	set ((DDRMC_Type *)0x400AE000)->PHY16 = 0x00002613

	set ((DDRMC_Type *)0x400AE000)->PHY01 = 0x00002615
	set ((DDRMC_Type *)0x400AE000)->PHY17 = 0x00002615

	set ((DDRMC_Type *)0x400AE000)->PHY02 = 0x00210000
	set ((DDRMC_Type *)0x400AE000)->PHY18 = 0x00210000
	set ((DDRMC_Type *)0x400AE000)->PHY34 = 0x00210000

	set ((DDRMC_Type *)0x400AE000)->PHY03 = 0x0001012a
	set ((DDRMC_Type *)0x400AE000)->PHY19 = 0x0001012a
	set ((DDRMC_Type *)0x400AE000)->PHY35 = 0x0001012a

	set ((DDRMC_Type *)0x400AE000)->PHY04 = 0x00002400
	set ((DDRMC_Type *)0x400AE000)->PHY20 = 0x00002400
	set ((DDRMC_Type *)0x400AE000)->PHY36 = 0x00002400

	set ((DDRMC_Type *)0x400AE000)->PHY49 = 0x00000000
	set ((DDRMC_Type *)0x400AE000)->PHY50 = 0x00001100
	set ((DDRMC_Type *)0x400AE000)->PHY52 = 0x00010101

	set ((DDRMC_Type *)0x400AE000)->CR00 = 0x00000601

	set ((DDRMC_Type *)0x400AE000)->CR69 = 0x00000200
end

document ddr_ctrl_init
Initialize DDR RAM controller so program can be loaded to the external RAM
end