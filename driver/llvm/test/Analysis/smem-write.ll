; ModuleID = 'smem-write.c'
source_filename = "smem-write.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.hfi_cmd_fake = type { i32 }

@smem = common global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @send_packet(i8* %pkt) #0 {
entry:
  %pkt.addr = alloca i8*, align 8
  store i8* %pkt, i8** %pkt.addr, align 8
  %0 = load i8*, i8** %pkt.addr, align 8
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast (i32* @smem to i8*), i8* %0, i64 4, i32 1, i1 false)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %pkt = alloca %struct.hfi_cmd_fake, align 4
  store i32 0, i32* %retval, align 4
  store i32 1, i32* @smem, align 4
  %size = getelementptr inbounds %struct.hfi_cmd_fake, %struct.hfi_cmd_fake* %pkt, i32 0, i32 0
  store i32 5, i32* %size, align 4
  %0 = bitcast %struct.hfi_cmd_fake* %pkt to i8*
  call void @send_packet(i8* %0)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (tags/RELEASE_500/final)"}
